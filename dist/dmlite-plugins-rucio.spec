Name:		dmlite-plugins-rucio
Version:	0.1.0
Release:	1%{?dist}
Summary:	Rucio plug-in for dmlite
Group:		Applications/Internet
License:	ASL 2.0
URL:		https://github.com/mlassnig/dmlite_plugin_rucio
Source0:	%{name}-%{version}.tar.gz
Buildroot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	cmake
BuildRequires:	dmlite-devel >= 0.6.0
%if %{?fedora}%{!?fedora:0} >= 10 || %{?rhel}%{!?rhel:0} >= 6
BuildRequires:  libcurl-devel
%else
BuildRequires:  curl-devel
%endif

%description
This package provides the Rucio plug-in for dmlite.

%prep
%setup -q -n %{name}-%{version}

%build
%cmake . -DCMAKE_INSTALL_PREFIX=/

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}

make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_libdir}/dmlite/plugin_rucio.so
%doc LICENSE README RELEASE-NOTES
%config(noreplace) %{_sysconfdir}/dmlite.conf.d/*

%changelog
