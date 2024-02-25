for module in `ninja -C out/Default -k0 | grep FAILED | awk '{print $2}'`
do
  echo "Replacing sysroot in $module"
  sed -i 's/--sysroot=..\/..\/build\/linux\/debian_bullseye_amd64-sysroot/--sysroot=\//g' `find out/Default/obj -name "${module}.ninja"`
done
