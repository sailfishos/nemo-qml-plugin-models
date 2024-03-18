Name:       nemo-qml-plugin-models-qt5

Summary:    Nemo QML models plugin
Version:    0.2.4
Release:    1
License:    BSD and LGPLv2+
URL:        https://github.com/sailfishos/nemo-qml-plugin-models
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Test)
BuildRequires:  pkgconfig(mlocale5)
BuildRequires:  pkgconfig(mlite5)

%description
%{summary}.

%package tests
Summary:    Nemo QML models plugin tests
Requires:   %{name} = %{version}-%{release}

%description tests
%{summary}.

%package devel
Summary:    Nemo QML models library headers
Requires:   %{name} = %{version}-%{release}

%description devel
%{summary}.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 VERSION=%{version}

make %{?_smp_mflags}

%install
%qmake5_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%license LICENSE.BSD LICENSE.LGPL
%{_libdir}/libnemomodels-qt5.so*
%{_libdir}/qt5/qml/org/nemomobile/models/libnemomodels.so
%{_libdir}/qt5/qml/org/nemomobile/models/qmldir
%{_libdir}/qt5/qml/org/nemomobile/models/plugins.qmltypes

%files tests
%defattr(-,root,root,-)
/opt/tests/nemo-qml-plugins/models/

%files devel
%defattr(-,root,root,-)
%{_includedir}/nemomodels-qt5/*
%{_libdir}/pkgconfig/nemomodels-qt5.pc
%{_libdir}/libnemomodels-qt5.prl
