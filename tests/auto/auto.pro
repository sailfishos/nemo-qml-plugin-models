TEMPLATE = aux

QML_FILES =  tst_*.qml
OTHER_FILES += $${QML_FILES}

target.files = $${QML_FILES}
target.path = /opt/tests/nemo-qml-plugins/models/auto

INSTALLS += target
