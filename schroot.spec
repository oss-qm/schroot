%global release_date	0
%global release_version	1.7.999

Summary: chroot management tool
Name: schroot
Version: 1.7.999
Release: 3%{?dist}
License: GPLv3
Group: Administration
Source: schroot-%{version}.tar.gz

BuildRequires:   cmake >= 2.6.2
BuildRequires:   gcc-c++
BuildRequires:   boost-devel
BuildRequires:   gettext-tools
BuildRequires:   pam-devel
BuildRequires:   groff

Requires: schroot-libs
Requires: schroot-common

%description
chroot management tool

%prep
%setup -q

%build
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DGIT_RELEASE_ENABLE=OFF \
    -DGIT_RELEASE_VERSION=%release_version \
    -DGIT_RELEASE_DATE_UNIX=%release_date \
    -Dtest=OFF \
    -Dnls=ON \
    -Dpam=ON \
    -Dlvm-snapshot=ON \
    -Dbtrfs-snapshot=ON \
    -Dblock-device=ON \
    -Dloopback=ON \
    -Dunion=ON \
    -Dunshare=ON \
    -Dpersonality=ON \
    -DCMAKE_INSTALL_PREFIX=%_prefix \
    -DCMAKE_INSTALL_SYSCONFDIR=%_sysconfdir \
    -DCMAKE_INSTALL_LOCALSTATEDIR=%_localstatedir \
    -DCMAKE_INSTALL_LIBEXECDIR=%_libexecdir \
    ..

make %{?_smp_mflags}

%install
%__rm -rf %buildroot

pushd build
make install DESTDIR=%buildroot
popd

for profile in default buildd desktop sbuild ; do
    echo "creating profile: $profile"
    mkdir -p %buildroot/%_sysconfdir/schroot/$profile
    for platform in all gnu linux sles ; do
        if [ -d etc/profile-templates/all/$platform ]; then
            echo "copying all/$platform to profile $profile"
            cp etc/profile-templates/all/$platform/* %buildroot/%_sysconfdir/schroot/$profile
        fi
    done
    for platform in all gnu linux sles ; do
        if [ -d etc/profile-templates/$profile/$platform ]; then
            echo "copying $profile/$platform to profile $profile"
            cp etc/profile-templates/$profile/$platform/* %buildroot/%_sysconfdir/schroot/$profile
        fi
    done
done

%post

%preun

%files
%defattr(-,root,root)
%_sysconfdir/bash_completion.d/schroot
%_bindir/*

%clean
%__rm -rf %buildroot

%package devel
Summary: schroot development files
Requires: schroot-libs

%description devel
schroot development files

%files devel
%_includedir/schroot/*.tcc
%_includedir/schroot/*.h
%_includedir/schroot/*/*.h
%_includedir/schroot/*/*/*.h
%_libdir/*.so
%_libdir/pkgconfig/*.pc

%package libs
Summary: schroot libraries
Requires: schroot-common

%description libs
schroot libraries

%files libs
%_libdir/*.so.*
%_libexecdir/schroot/*

%package common
Summary: architecture independent files for schroot
BuildArch: noarch

%description common
architecture independent files for schroot

%files common
%dir %_sysconfdir/schroot
%dir %_localstatedir/lib/schroot
%dir %_localstatedir/lib/schroot/mount
%dir %_localstatedir/lib/schroot/session
%dir %_localstatedir/lib/schroot/unpack
%dir %_localstatedir/lib/schroot/union
%dir %_localstatedir/lib/schroot/union/overlay
%dir %_localstatedir/lib/schroot/union/underlay
%_libexecdir/schroot/*
%_datadir/schroot/*
%_datadir/locale/*/LC_MESSAGES/*.mo
%_mandir/man*/*.gz
%_sysconfdir/schroot/*
%_sysconfdir/schroot/default/*
%_sysconfdir/pam.d/*
%doc AUTHORS COPYING NEWS THANKS HACKING README TODO

%changelog
* Mon Feb 24 2020 Enrico Weigelt, metux IT consult <info@metux.net> - 1.7.999-1
- New package for SLES12
