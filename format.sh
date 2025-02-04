git ls-files -- '*.cpp' '*.h' | xargs clang-format -i -style=file
git diff --exit-code --color
