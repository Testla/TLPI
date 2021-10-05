#!/usr/bin/python3
import os
import stat
import subprocess
import tempfile
import typing


def one_bit_unset(x: int) -> typing.Iterator[int]:
    """ Returns all integers obtained by unsetting a one bit of x. """
    copy = x
    while copy:
        lowest_one_bit = (copy & (copy - 1)) ^ copy
        yield x ^ lowest_one_bit
        copy ^= lowest_one_bit


def test_operation(
    operation: typing.Callable,
    arguments: typing.Sequence[str],
    expect_permission_error: bool
) -> bool:
    try:
        operation(*arguments)
    except PermissionError:
        permission_error_occured = True
    else:
        permission_error_occured = False
    return (permission_error_occured == expect_permission_error)


def change_mode_root(mode: int, path: str) -> None:
    subprocess.check_call(['sudo', 'chmod', f'{mode:04o}'[-4:], path])


def verify_minimum_permission(
        operation: typing.Callable,
        arguments: typing.Sequence[str],
        minimum_permissions: typing.Sequence[int],
        use_root_and_path: bool = False,
        paths_going_to_disappear: typing.Optional[typing.Set[int]] = None,
        expect_not_enough_permission: bool = False,
    ) -> None:
    # A set of permissions is minimum if and only if the operation can be carried
    # out with it, but can not if any one bit is unset.
    # WARNING: To deal with various needs while leaving as much arguments (thus permissions) as
    # possible, this function is now quite messy.

    origin_permissions = [os.stat(argument).st_mode for argument in arguments]
    if use_root_and_path:
        def change_mode(index: int, mode: int) -> None:
            change_mode_root(mode, arguments[index])
    else:
        # When we remove the execute permission of a directory, we will not be able to change
        # permission of the file. So we first get the file descriptors.
        fds = [os.open(argument, os.O_RDONLY) for argument in arguments]
        def change_mode(index: int, mode: int) -> None:
            os.chmod(fds[index], mode)

    # Initalize to minimum permissions.
    for index, permission in enumerate(minimum_permissions):
        change_mode(index, permission)

    if not expect_not_enough_permission:
        for index, permission in enumerate(minimum_permissions):
            for lesser_permission in one_bit_unset(permission):
                change_mode(index, lesser_permission)
                assert test_operation(operation, arguments, True), f'{index} {lesser_permission:04o}'
            change_mode(index, permission)

    assert test_operation(operation, arguments, expect_not_enough_permission)

    for index, origin_permission in enumerate(origin_permissions):
        if not (
                use_root_and_path
                and paths_going_to_disappear
                and index in paths_going_to_disappear):
            change_mode(index, origin_permission)
        if not use_root_and_path:
            os.close(fds[index])


def change_owner_root(user: str, target: str) -> None:
    subprocess.check_call(['sudo', 'chown', user, target])


def main() -> None:
    # Thanks to Python library, the newly created temporary directory and all
    # its contents will be automatically removed.
    with tempfile.TemporaryDirectory() as dir1,\
            tempfile.TemporaryDirectory() as dir2:
        def create_a_new_file(directory: str) -> None:
            tempfile.TemporaryFile(dir=directory)
        verify_minimum_permission(create_a_new_file, [dir1], [stat.S_IWUSR | stat.S_IXUSR])

        # Note that we intentionally place the delete test at the end and specify delete=False.
        with tempfile.NamedTemporaryFile(dir=dir1, delete=False) as f1:
            def open_a_file_for_reading(_: str, file: str) -> None:
                with open(file, 'r'):
                    pass
            verify_minimum_permission(
                open_a_file_for_reading,
                [dir1, f1.name],
                [stat.S_IXUSR, stat.S_IRUSR])

            def open_a_file_for_writing(_: str, file: str) -> None:
                with open(file, 'w'):
                    pass
            verify_minimum_permission(
                open_a_file_for_writing,
                [dir1, f1.name],
                [stat.S_IXUSR, stat.S_IWUSR])

            def delete_a_file(_: str, file: str) -> None:
                os.remove(file)
            verify_minimum_permission(
                delete_a_file,
                [dir1, f1.name],
                [stat.S_IWUSR | stat.S_IXUSR, 0],
            )

        def rename_a_file(_: str, target_directory: str, file: str) -> None:
            os.rename(file, os.path.join(target_directory, os.path.basename(file)))

        # Specify delete=False because the file will be renamed.
        with tempfile.NamedTemporaryFile(dir=dir1, delete=False) as f1:
            verify_minimum_permission(
                rename_a_file,
                [dir1, dir2, f1.name],
                [
                    stat.S_IWUSR | stat.S_IXUSR,
                    stat.S_IWUSR | stat.S_IXUSR,
                    0,
                ],
            )

        with tempfile.NamedTemporaryFile(dir=dir1, delete=False) as f1,\
                open(os.path.join(dir2, os.path.basename(f1.name)), 'w') as f2:
            def rename_a_file_target_exists(_: str, __: str, file: str, target_file: str) -> None:
                os.rename(file, target_file)
            verify_minimum_permission(
                rename_a_file_target_exists,
                [dir1, dir2, f1.name, f2.name],
                [
                    stat.S_IWUSR | stat.S_IXUSR,
                    stat.S_IWUSR | stat.S_IXUSR,
                    0,
                    0,
                ],
            )

        # When a directory has sticky bit set, removing or renaming a file requires
        # write permission on the directory and ownership of either the file or the directory.
        # If target of rename already exists, it's like removing that file first.
        # One problem is that when we change the owner for testing, we will not be able to
        # change the mode as normal user.

        saved_stat = os.stat(dir1)

        def delete_a_file_in_sticky_directory(dir: str, file: str) -> None:
            change_mode_root(os.stat(dir).st_mode | stat.S_ISVTX, dir)
            os.remove(file)

        with tempfile.NamedTemporaryFile(dir=dir1, delete=False) as f1:
            # Because we change only owner but not group of the directory, category of permission is
            # group instead of other.
            verify_args = (
                delete_a_file_in_sticky_directory,
                [dir1, f1.name],
                [stat.S_IWGRP | stat.S_IXGRP, 0],
            )
            verify_kwargs = {
                'use_root_and_path': True,
            }

            # Grant some permission so that we can call os.stat on f1 at first.
            os.chmod(dir1, saved_stat.st_mode | (stat.S_IWGRP | stat.S_IXGRP))

            change_owner_root('root', dir1)
            change_owner_root('root', f1.name)
            verify_minimum_permission(
                *verify_args,
                **verify_kwargs,
                expect_not_enough_permission=True,
            )

            change_owner_root(str(saved_stat.st_uid), f1.name)
            verify_minimum_permission(
                *verify_args,
                **verify_kwargs,
                paths_going_to_disappear={1},
            )

            change_owner_root(str(saved_stat.st_uid), dir1)
            os.chmod(dir1, saved_stat.st_mode)

        with tempfile.NamedTemporaryFile(dir=dir1, delete=False) as f1:
            change_owner_root('root', f1.name)
            verify_minimum_permission(
                delete_a_file_in_sticky_directory,
                [dir1, f1.name],
                [stat.S_IWUSR | stat.S_IXUSR, 0],
                use_root_and_path=True,
                paths_going_to_disappear={1},
            )

        # Rename is like delete(I guess), we just need to be able to delete source and
        # existing target(if any).
        # To not make the code too long, I will only test one case, that source and target are in
        # the same directory, which user is not owner of, and user needs to be owner of both file.
        with tempfile.NamedTemporaryFile(dir=dir1, delete=False) as f1,\
                tempfile.NamedTemporaryFile(dir=dir1, delete=False) as f2:
            # Grant some permission so that we can call os.stat on f1 at first.
            os.chmod(dir1, saved_stat.st_mode | (stat.S_IWGRP | stat.S_IXGRP))
            change_owner_root('root', dir1)

            def rename_a_file_target_exists_in_sticky_directory(dir: str, source: str, target: str) -> None:
                change_mode_root(os.stat(dir).st_mode | stat.S_ISVTX, dir)
                os.rename(source, target)

            verify_args = (
                rename_a_file_target_exists_in_sticky_directory,
                [dir1, f1.name, f2.name],
                [stat.S_IWGRP | stat.S_IXGRP, 0, 0],
            )
            verify_kwargs = {
                'use_root_and_path': True,
            }

            change_owner_root('root', f1.name)
            verify_minimum_permission(
                *verify_args,
                **verify_kwargs,
                expect_not_enough_permission=True,
            )

            change_owner_root(str(saved_stat.st_uid), f1.name)
            change_owner_root('root', f2.name)
            verify_minimum_permission(
                *verify_args,
                **verify_kwargs,
                expect_not_enough_permission=True,
            )

            change_owner_root(str(saved_stat.st_uid), f2.name)
            verify_minimum_permission(
                *verify_args,
                **verify_kwargs,
                paths_going_to_disappear={1},
            )

            change_owner_root(str(saved_stat.st_uid), dir1)
            os.chmod(dir1, saved_stat.st_mode)


if __name__ == '__main__':
    main()
