install dependencies:
ant \
batik \
gcc \
java \
g++ \
make \
expat \
libexpat1-dev \
zlib1g-dev \

configure batik path in jtile/build.xml

git submodule init
git submodule update

build each directory 
=> ant
make


overpass
pushd src/
autoreconf
libtoolize
automake --add-missing
autoreconf
popd
cd build
../src/configure CXXFLAGS="-Wall -O2" --prefix=/export/home/bernd/opt/overpass
make install -j

populate world
osmconvert europe-latest.osm.pbf -b=8.8588,47.4736,9.7617,47.8316 -o=bodensee.osm

