TEMPLATE = aux

QML_FILES =  auto/tst_*.qml
OTHER_FILES += $${QML_FILES}

target.files = $${QML_FILES}
target.path = /opt/tests/nemo-qml-plugins/models/auto

definition.files = tests.xml
definition.path = /opt/tests/nemo-qml-plugins/models/test-definition

INSTALLS += target definition
