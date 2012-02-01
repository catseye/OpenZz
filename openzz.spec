Name:		openzz
Summary:	An Interactive Dynamic parser for Incremental Grammars
%define version 1.0.4
Version:	%{version}
Release:	4
License:	GPL
Group:		Development
BuildRequires:	readline-devel libtermcap-devel
Source:		http://prdownloads.sourceforge.net/openzz/openzz-%{version}-%{release}.tar.gz
BuildRoot:	%_tmppath/%{name}-%{version}-%{release}-root

%description 
OpenZz is a dynamic parser which allows its grammar to be
extended by commands written in its own Zz language. Due to the
interpreted and dynamic nature of the parser OpenZz can be used to
develop both fast language prototypes and full compilers.

%prep
%setup -q

%build
./configure --prefix=%{_prefix}
make CDEBUGFLAGS="$RPM_OPT_FLAGS"

%install
make DESTDIR=$RPM_BUILD_ROOT install
mkdir -p %_tmppath/%name-%version-%release-root/usr/share/doc/%name-%version
cp doc/zzdoc.html %_tmppath/%name-%version-%release-root/%{_prefix}/share/doc/%name-%version

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun

%files
%defattr(-,root,root)
%{_prefix}/bin/ozz
%{_prefix}/lib/libozz.so.0.1.4
%{_prefix}/lib/libozz.la
%{_prefix}/lib/libozz.a
%{_prefix}/lib/libozzi.so.0.1.4
%{_prefix}/lib/libozzi.la
%{_prefix}/lib/libozzi.a
%{_prefix}/include/ozz/zz.h
%{_prefix}/include/ozz/zzbind.h
%doc %{_prefix}/share/doc/openzz-%version/zzdoc.html

