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
    property int lastError

    ListModel {
        id: baseModel

        Component.onCompleted: {
            // Populate dynamically to permit list values
            append({
                order: 1,
                name: 'Alice',
                gender: 'female',
                pets: [ 'cat', 'dog', 'goldfish' ]
            })
            append({
                order: 2,
                name: 'Andy',
                gender: 'male',
                pets: []
            })
            append({
                order: 3,
                name: 'Antonio',
                gender: 'male',
                pets: [ 'dog', 'hamster' ]
            })
            append({
                order: 4,
                name: 'Antti',
                gender: 'male',
                pets: [ 'cat' ]
            })
            append({
                order: 5,
                name: 'Bob',
                gender: 'male',
                pets: [ 'cat', 'iguana' ]
            })
        }
    }

    ListModel {
        id: otherModel

        ListElement {
            category: 'soldier'
            order: 'march'
        }
        ListElement {
            category: 'soldier'
            order: 'guard'
        }
        ListElement {
            category: 'officer'
            order: 'observe'
        }
    }

    ListModel {
        id: refineModel

        ListElement {
            name: 'abcdez'
            order: 1
        }
        ListElement {
            name: 'abcz'
            order: 2
        }
        ListElement {
            name: 'abcdz'
            order: 3
        }
        ListElement {
            name: 'abz'
            order: 4
        }
        ListElement {
            name: 'az'
            order: 5
        }
        ListElement {
            name: 'abcdez'
            order: 6
        }
        ListElement {
            name: 'abcdefz'
            order: 7
        }
        ListElement {
            name: 'az'
            order: 8
        }
        ListElement {
            name: 'abcdefgz'
            order: 9
        }
        ListElement {
            name: 'abz'
            order: 10
        }
    }

    SearchModel {
        id: searchModel
    }

    Repeater {
        id: repeater

        delegate: Item {
            property int orderValue: order
            property string nameValue: name
            property string genderValue: gender
            property string categoryValue: category
            property string orderText: order
        }
    }

    resources: TestCase {
        name: "FileModel"

        function init() {
        }
        function cleanup() {
        }

        function test_a_unused() {
            compare(searchModel.populated, false)
            compare(searchModel.count, 0)
            compare(searchModel.pattern, '')
            compare(searchModel.caseSensitivity, Qt.CaseSensitive)
        }

        function test_b_unfiltered() {
            searchModel.sourceModel = baseModel
            compare(searchModel.populated, true)
            compare(searchModel.count, 5)
            compare(repeater.count, 0)

            repeater.model = searchModel
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(2).orderValue, 3)
            compare(repeater.itemAt(4).nameValue, 'Bob')

            repeater.model = null
            compare(repeater.count, 0)

            searchModel.sourceModel = null
            compare(searchModel.populated, false)
            compare(searchModel.count, 0)

            repeater.model = searchModel
            compare(repeater.count, 0)

            searchModel.sourceModel = otherModel
            compare(searchModel.populated, true)
            compare(searchModel.count, 3)
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).categoryValue, 'soldier')
            compare(repeater.itemAt(1).orderText, 'guard')
            compare(repeater.itemAt(2).nameValue, '')

            repeater.model = null
            compare(repeater.count, 0)
        }

        function test_c_search() {
            repeater.model = null
            compare(repeater.count, 0)

            searchModel.sourceModel = baseModel
            compare(searchModel.populated, true)
            compare(searchModel.count, 5)

            repeater.model = searchModel
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(4).nameValue, 'Bob')

            searchModel.pattern = 'An'
            compare(repeater.count, 0)

            searchModel.searchRoles = [ 'name' ]
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Andy')
            compare(repeater.itemAt(2).nameValue, 'Antti')

            searchModel.pattern = 'AN'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Andy')
            compare(repeater.itemAt(2).nameValue, 'Antti')

            searchModel.pattern = 'maLE'
            compare(repeater.count, 0)

            searchModel.searchRoles = [ 'name', 'gender' ]
            compare(repeater.count, 4)
            compare(repeater.itemAt(0).nameValue, 'Andy')
            compare(repeater.itemAt(3).nameValue, 'Bob')

            searchModel.caseSensitivity = Qt.CaseSensitive
            compare(repeater.count, 0)

            searchModel.searchRoles = [ 'order' ]
            compare(repeater.count, 0)

            searchModel.pattern = '2'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).nameValue, 'Andy')

            /* TODO: Does not work with ListElement members...
            */

            repeater.model = null
            searchModel.sourceModel = otherModel
            repeater.model = searchModel
            compare(repeater.count, 0)

            searchModel.pattern = 'guard'
            compare(repeater.count, 1)

            searchModel.pattern = 'obs'
            compare(repeater.count, 1)
        }

        function test_d_multi_search() {
            repeater.model = null
            compare(repeater.count, 0)

            searchModel.pattern = ''
            searchModel.sourceModel = baseModel
            compare(searchModel.populated, true)
            compare(searchModel.count, 5)

            repeater.model = searchModel
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(4).nameValue, 'Bob')

            searchModel.pattern = 'An mail'
            compare(repeater.count, 0)

            searchModel.searchRoles = [ 'name' ]
            compare(repeater.count, 0)

            searchModel.pattern = 'An male'
            compare(repeater.count, 0)

            searchModel.searchRoles = [ 'name', 'gender' ]
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Andy')
            compare(repeater.itemAt(2).nameValue, 'Antti')
        }

        function test_e_refine() {
            repeater.model = null
            compare(repeater.count, 0)

            searchModel.sourceModel = null
            searchModel.searchRoles = []
            searchModel.pattern = ''
            searchModel.caseSensitivity = Qt.CaseSensitive

            searchModel.sourceModel = refineModel
            compare(searchModel.populated, true)
            compare(searchModel.count, 10)

            repeater.model = searchModel

            searchModel.searchRoles = [ 'name' ]
            searchModel.pattern = 'a'
            compare(repeater.count, 10)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(9).orderValue, 10)

            searchModel.pattern = 'ab'
            compare(repeater.count, 8)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(7).orderValue, 10)

            searchModel.pattern = 'abcd'
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(4).orderValue, 9)

            searchModel.pattern = 'abcdef'
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 7)
            compare(repeater.itemAt(1).orderValue, 9)

            searchModel.pattern = 'abcdefgh'
            compare(repeater.count, 0)

            searchModel.pattern = 'abcdefg'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 9)

            searchModel.pattern = 'abcde'
            compare(repeater.count, 4)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(3).orderValue, 9)

            searchModel.pattern = 'ab'
            compare(repeater.count, 8)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(7).orderValue, 10)

            searchModel.pattern = 'a'
            compare(repeater.count, 10)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(9).orderValue, 10)

            searchModel.pattern = 'abc'
            compare(repeater.count, 6)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(5).orderValue, 9)

            searchModel.pattern = 'bcd'
            compare(repeater.count, 0)

            searchModel.pattern = 'abc'
            compare(repeater.count, 6)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(5).orderValue, 9)
        }
    }
}
