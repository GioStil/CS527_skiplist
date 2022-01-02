# Setup Development Environment

## Pre commit hooks using pre-commit

To install pre-commit:

	pip3 install pre-commit --user
	pre-commit --version
	2.2.0

To install pre-commit hooks:

	cd parallax
	pre-commit install
    pre-commit install --hook-type commit-msg

If everything worked as it should then the following message should be printed:

    pre-commit installed at .git/hooks/pre-commit

If you want to run a specific hook with a specific file run:

	pre-commit run hook-id --files filename

For example:

	pre-commit run cmake-format --files CMakeLists.txt

## Commit message template

To set up the commit template you need to run:

	git config commit.template .git-commit-template

## Debugging with Compiler Sanitizers

If you want to learn what compilers sanitizers are, check [Wikipedia](https://en.wikipedia.org/wiki/AddressSanitizer).

To use compiler sanitizers to debug code in this project you only need to activate when running the `cmake` command by adding `-DUSE_SANITIZER=YourSanitizer`.
For example to compile your code with the leak sanitizer:

	cmake3 .. -DUSE_SANITIZER=Leak

To see which sanitizers are supported check this [file](https://github.com/StableCoder/cmake-scripts/blob/main/sanitizers.cmake#L20).
