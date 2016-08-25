/*
 * Copyright (C) 2016 Jolla Ltd.
 * Contact: Matt Vogt <matthew.vogt@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Jolla Ltd. nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

import QtTest 1.0
import QtQuick 2.0
import org.nemomobile.models 1.0

Item {
    Component {
        id: item

        QtObject {
            property string name
        }
    }

    ObjectListModel {
        id: firstModel
        objectName: 'first'
        automaticRoles: true

        Component.onCompleted: {
            appendItem(item.createObject(this, { name: 'Alice' }))
        }
    }

    ObjectListModel {
        id: secondModel
        objectName: 'second'
        automaticRoles: true

        Component.onCompleted: {
            appendItem(item.createObject(this, { name: 'Andy' }))
            appendItem(item.createObject(this, { name: 'Antonio' }))
        }
    }

    ObjectListModel {
        id: thirdModel
        objectName: 'third'
        automaticRoles: true

        Component.onCompleted: {
            appendItem(item.createObject(this, { name: 'Antti' }))
            appendItem(item.createObject(this, { name: 'Bob' }))
        }
    }

    CompositeModel {
        id: compositeModel
    }

    Repeater {
        id: repeater

        delegate: Item {
            property string nameValue: name
            property string sourceModelValue: sourceModel
        }
    }

    resources: TestCase {
        name: "CompositeModel"

        function init() {
        }
        function cleanup() {
        }

        function test_a_models() {
            repeater.model = null
            compare(repeater.count, 0)

            compare(firstModel.populated, true)
            compare(firstModel.count, 1)

            compare(secondModel.populated, true)
            compare(secondModel.count, 2)

            compare(thirdModel.populated, true)
            compare(thirdModel.count, 2)

            compare(thirdModel.populated, true)
            compare(thirdModel.count, 2)

            compare(compositeModel.populated, false)
            compare(compositeModel.count, 0)

            repeater.model = firstModel
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).sourceModelValue, '')
            compare(repeater.itemAt(0).nameValue, 'Alice')

            repeater.model = secondModel
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).sourceModelValue, '')
            compare(repeater.itemAt(0).nameValue, 'Andy')
            compare(repeater.itemAt(1).nameValue, 'Antonio')

            repeater.model = thirdModel
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).sourceModelValue, '')
            compare(repeater.itemAt(0).nameValue, 'Antti')
            compare(repeater.itemAt(1).nameValue, 'Bob')

            compositeModel.models = [ firstModel ]
            compare(compositeModel.populated, true)

            repeater.model = compositeModel
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).sourceModelValue, 'first')
            compare(repeater.itemAt(0).nameValue, 'Alice')

            repeater.model = null
            compositeModel.models = [ secondModel ]
            repeater.model = compositeModel
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).sourceModelValue, 'second')
            compare(repeater.itemAt(0).nameValue, 'Andy')
            compare(repeater.itemAt(1).nameValue, 'Antonio')

            repeater.model = null
            compositeModel.models = [ thirdModel ]
            repeater.model = compositeModel
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).sourceModelValue, 'third')
            compare(repeater.itemAt(0).nameValue, 'Antti')
            compare(repeater.itemAt(1).nameValue, 'Bob')

            repeater.model = null
            compositeModel.models = [ firstModel, secondModel, thirdModel ]
            repeater.model = compositeModel
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).sourceModelValue, 'first')
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(1).sourceModelValue, 'second')
            compare(repeater.itemAt(1).nameValue, 'Andy')
            compare(repeater.itemAt(2).sourceModelValue, 'second')
            compare(repeater.itemAt(2).nameValue, 'Antonio')
            compare(repeater.itemAt(3).sourceModelValue, 'third')
            compare(repeater.itemAt(3).nameValue, 'Antti')
            compare(repeater.itemAt(4).sourceModelValue, 'third')
            compare(repeater.itemAt(4).nameValue, 'Bob')

            repeater.model = null
            compositeModel.models = [ thirdModel, secondModel, firstModel ]
            repeater.model = compositeModel
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).sourceModelValue, 'third')
            compare(repeater.itemAt(0).nameValue, 'Antti')
            compare(repeater.itemAt(1).sourceModelValue, 'third')
            compare(repeater.itemAt(1).nameValue, 'Bob')
            compare(repeater.itemAt(2).sourceModelValue, 'second')
            compare(repeater.itemAt(2).nameValue, 'Andy')
            compare(repeater.itemAt(3).sourceModelValue, 'second')
            compare(repeater.itemAt(3).nameValue, 'Antonio')
            compare(repeater.itemAt(4).sourceModelValue, 'first')
            compare(repeater.itemAt(4).nameValue, 'Alice')
        }
    }
}
