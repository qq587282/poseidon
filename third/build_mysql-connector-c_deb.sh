#!/bin/bash

set -e

pkgname="libmysqlclient"
pkgversion="6.1.6"
pkglicense="GPL or Commercial (https://www.mysql.com/about/legal/licensing/oem/)"
pkggroup="https://dev.mysql.com/"
pkgsource="https://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-${pkgversion}-src.tar.gz"
maintainer="lh_mouse"
provides="libmysqlclient"

dstdir="$(pwd)"
tempdir="$(mktemp -d)"

if [[ $EUID -ne 0 ]]; then
	echo "You must run this script as root."
	exit 1
fi

[[ -z "${tempdir}" ]] || rm -rf "${tempdir}"
mkdir -p "${tempdir}"
trap "rm -rf \"${tempdir}\"" EXIT
cd "${tempdir}"

_archive="$(basename -- ${pkgsource})"
wget -O "${_archive}" "${pkgsource}"
tar -xzvf "${_archive}"

cd "$(basename "${_archive}" ".tar.gz")"
CFLAGS="-O3" cmake . -DDEFAULT_CHARSET=utf8 -DDEFAULT_COLLATION=utf8_general_ci -DCMAKE_INSTALL_PREFIX=/usr/local
make -j4
checkinstall -y --pkgname="${pkgname}" --pkgversion="${pkgversion}" --pkglicense="${pkglicense}" --pkggroup="${pkggroup}"	\
	--pkgsource="${pkgsource}" --maintainer="${maintainer}" --provides="${provides}"
mv *.deb "${dstdir}"/