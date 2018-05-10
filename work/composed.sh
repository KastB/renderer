#!/bin/bash
#getworld
mkdir tmp cache tiles
echo "getworld"
echo "[timeout:3600];(rel['seamark:type'];>;way['seamark:type'];>;node['seamark:type'];);out meta;" | ~/opt/overpass/bin/osm3s_query --db-dir=/home/renderaccount/overpass_db/ > next.osm 2> errors.txt
echo "getworld - done"

touch world.osm
diff world.osm next.osm | grep id= | grep -v "<tag" > diffs
java -jar ../jsearch/jsearch.jar ./

#tilegen
echo "tilegen"
for file in $(ls tmp | grep "\.osm"); do
  tx=$(echo $file | cut -f 1 -d'-')
  ty=$(echo $file | cut -f 2 -d'-')
  z=$(echo $file | cut -f 3 -d'-')
  z=$(echo $z | cut -f 1 -d'.')
  if [ $z = 12 ]; then
	for k in {12..18}; do
	  ../searender/searender ../searender/symbols/symbols.defs $k >tmp/$tx-$ty-$k.svg <tmp/$file
	done;
  else
	../searender/searender ../searender/symbols/symbols.defs $z >tmp/$tx-$ty-$z.svg <tmp/$file
  fi
  rm tmp/$file
  
  java -jar ../jtile/jtile.jar tmp/ tiles/ $z $tx $ty
  echo "$(date) rendering $z $tx $ty" >> log.txt
done
echo "tilegen - done"

#tiler
echo "tiler"
for file in $(ls cache); do
	tile=$(echo $file | sed -e s?-?/?g)
	mkdir -p $(dirname $tile)
	mv -f cache/$file $tile
done

echo "tiler done"
