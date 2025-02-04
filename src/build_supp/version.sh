echo "#include <string>" > version.cpp
echo "const std::string ANTHRACITE_VERSION_STRING = \"$(cat version.txt)\";" >> version.cpp
echo "const std::string ANTHRACITE_FULL_VERSION_STRING = \"Anthracite/$(cat version.txt)\";" >> version.cpp
