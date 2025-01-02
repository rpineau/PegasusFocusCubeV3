#!/bin/bash

PACKAGE_NAME="PegasusFocusCubeV3_X2.pkg"
BUNDLE_NAME="org.rti-zone.PegasusFocusCubeV3X2"

if [ ! -z "$app_id_signature" ]; then
    codesign -f -s "$app_id_signature" --verbose ../build/Release/libPegasusFocusCubeV3.dylib
fi

mkdir -p ROOT/tmp/PegasusFocusCubeV3_X2/
cp "../PegasusFocusCubeV3.ui" ROOT/tmp/PegasusFocusCubeV3_X2/
cp "../PegasusAstroFCV3.png" ROOT/tmp/PegasusFocusCubeV3_X2/
cp "../focuserlist PegasusFocusCubeV3.txt" ROOT/tmp/PegasusFocusCubeV3_X2/
cp "../build/Release/libPegasusFocusCubeV3.dylib" ROOT/tmp/PegasusFocusCubeV3_X2/

if [ ! -z "$installer_signature" ]; then
	# signed package using env variable installer_signature
	pkgbuild --root ROOT --identifier $BUNDLE_NAME --sign "$installer_signature" --scripts Scripts --version 1.0 $PACKAGE_NAME
	pkgutil --check-signature ./${PACKAGE_NAME}

else
    pkgbuild --root ROOT --identifier $BUNDLE_NAME --scripts Scripts --version 1.0 $PACKAGE_NAME
fi

rm -rf ROOT
