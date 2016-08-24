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
            property real dimension
            property var pets
        }
    }

    ObjectListModel {
        id: objectListModel
        automaticRoles: true

        Component.onCompleted: {
            appendItem(item.createObject(this, {
                name: 'Alice',
                dimension: 1.65,
                pets: [ 'cat', 'dog', 'goldfish' ]
            }))
            appendItem(item.createObject(this, {
                name: 'Andy',
                dimension: 1.80,
                pets: []
            }))
            appendItem(item.createObject(this, {
                name: 'Antonio',
                dimension: 1.80,
                pets: [ 'dog', 'hamster' ]
            }))
            appendItem(item.createObject(this, {
                name: 'Antti',
                dimension: 2.10,
                pets: [ 'cat' ]
            }))
            appendItem(item.createObject(this, {
                name: 'Bob',
                dimension: 1.60,
                pets: [ 'cat', 'iguana' ]
            }))
        }
    }

    Repeater {
        id: repeater

        delegate: Item {
            property string nameValue: name
            property real dimensionValue: dimension
            property var petsValue: pets
        }
    }

    resources: TestCase {
        name: "ObjectListModel"

        function init() {
        }
        function cleanup() {
        }

        function test_a_roles() {
            repeater.model = null
            compare(repeater.count, 0)

            compare(objectListModel.populated, true)
            compare(objectListModel.count, 5)

            repeater.model = objectListModel
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(1).nameValue, 'Andy')
            compare(repeater.itemAt(2).nameValue, 'Antonio')
            compare(repeater.itemAt(3).nameValue, 'Antti')
            compare(repeater.itemAt(4).nameValue, 'Bob')

            compare(repeater.itemAt(0).dimensionValue, 1.65)
            compare(repeater.itemAt(1).dimensionValue, 1.80)
            compare(repeater.itemAt(2).dimensionValue, 1.80)
            compare(repeater.itemAt(3).dimensionValue, 2.10)
            compare(repeater.itemAt(4).dimensionValue, 1.60)

            compare(repeater.itemAt(0).petsValue, [ 'cat', 'dog', 'goldfish' ])
            compare(repeater.itemAt(1).petsValue, [])
            compare(repeater.itemAt(2).petsValue, [ 'dog', 'hamster' ])
            compare(repeater.itemAt(3).petsValue, [ 'cat' ])
            compare(repeater.itemAt(4).petsValue, [ 'cat', 'iguana' ])
        }
    }
}
