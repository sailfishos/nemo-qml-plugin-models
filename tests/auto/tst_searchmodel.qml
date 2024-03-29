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

        ListElement {
            order: 1
            name: 'Alice'
            gender: 'female'
        }
        ListElement {
            order: 2
            name: 'Andy'
            gender: 'male'
        }
        ListElement {
            order: 3
            name: 'Antonio'
            gender: 'male'
        }
        ListElement {
            order: 4
            name: 'Antti'
            gender: 'male'
        }
        ListElement {
            order: 5
            name: 'Bob'
            gender: 'male'
        }
    }

    Component {
        id: item

        QtObject {
            property string name
            property var pets
        }
    }

    ObjectListModel {
        id: objectListModel

        Component.onCompleted: {
            appendItem(item.createObject(this, {
                name: 'Alice',
                pets: [ 'cat', 'dog', 'goldfish' ]
            }))
            appendItem(item.createObject(this, {
                name: 'Andy',
                pets: []
            }))
            appendItem(item.createObject(this, {
                name: 'Antonio',
                pets: [ 'dog', 'hamster' ]
            }))
            appendItem(item.createObject(this, {
                name: 'Antti',
                pets: [ 'cat' ]
            }))
            appendItem(item.createObject(this, {
                name: 'Bob',
                pets: [ 'cat', 'iguana' ]
            }))
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
        ListElement {
            name: 'Abcdez'
            order: 11
        }
        ListElement {
            name: 'ABcDEfz'
            order: 12
        }
        ListElement {
            name: 'Az'
            order: 13
        }
        ListElement {
            name: 'AbCDefgz'
            order: 14
        }
        ListElement {
            name: 'ABz'
            order: 15
        }
    }

    ListModel {
        id: decompositionModel

        ListElement {
            name: 'Elvis'
            order: 1
        }
        ListElement {
            name: 'elvis'
            order: 2
        }
        ListElement {
            name: 'Ëlvis'
            order: 3
        }
        ListElement {
            name: 'Elviß'
            order: 4
        }
        ListElement {
            name: 'Ælvis'
            order: 5
        }
        ListElement {
            name: 'ælvis'
            order: 6
        }
        ListElement {
            name: 'Ølvis'
            order: 7
        }
        ListElement {
            name: 'Ølviß'
            order: 8
        }
    }

    ListModel {
        id: apostropheModel

        ListElement {
            name: 'D\'urbanville'
            order: 1
        }
        ListElement {
            name: 'Talkin\' Blues'
            order: 2
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

    Repeater {
        id: objectRepeater

        delegate: Item {
            property string nameValue: object.name
        }
    }

    resources: TestCase {
        name: "SearchModel"

        function init() {
        }
        function cleanup() {
        }

        function test_a_unused() {
            compare(searchModel.populated, false)
            compare(searchModel.count, 0)
            compare(searchModel.pattern, '')
            compare(searchModel.caseSensitivity, Qt.CaseSensitive)
            compare(searchModel.matchType, SearchModel.MatchBeginning)
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

            searchModel.searchRoles = [ 'name' ]
            compare(repeater.count, 0)

            searchModel.pattern = 'n'
            compare(repeater.count, 0)

            searchModel.matchType = SearchModel.MatchAnywhere
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Andy')
            compare(repeater.itemAt(2).nameValue, 'Antti')

            searchModel.pattern = 'nt'
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).nameValue, 'Antonio')
            compare(repeater.itemAt(1).nameValue, 'Antti')

            searchModel.pattern = 'ntt'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).nameValue, 'Antti')

            searchModel.pattern = 'nio'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).nameValue, 'Antonio')

            searchModel.matchType = SearchModel.MatchBeginning
            compare(repeater.count, 0)

            repeater.model = null

            searchModel.searchRoles = [ 'order' ]
            searchModel.sourceModel = otherModel

            repeater.model = searchModel
            compare(repeater.count, 0)

            searchModel.pattern = 'guard'
            compare(repeater.count, 1)

            searchModel.pattern = 'obs'
            compare(repeater.count, 1)

            searchModel.pattern = ''
            searchModel.searchRoles = []
            searchModel.searchProperties = [ 'pets' ]
            searchModel.sourceModel = objectListModel

            objectRepeater.model = searchModel
            compare(objectRepeater.count, 5)

            searchModel.pattern = 'ca'
            compare(objectRepeater.count, 3)
            compare(objectRepeater.itemAt(0).nameValue, 'Alice')
            compare(objectRepeater.itemAt(2).nameValue, 'Bob')

            searchModel.pattern = 'dog'
            compare(objectRepeater.count, 2)
            compare(objectRepeater.itemAt(0).nameValue, 'Alice')
            compare(objectRepeater.itemAt(1).nameValue, 'Antonio')

            searchModel.pattern = 'ster'
            compare(objectRepeater.count, 0)

            searchModel.matchType = SearchModel.MatchAnywhere
            compare(objectRepeater.count, 1)
            compare(objectRepeater.itemAt(0).nameValue, 'Antonio')

            searchModel.matchType = SearchModel.MatchBeginning
            compare(objectRepeater.count, 0)

            objectRepeater.model = null
            compare(objectRepeater.count, 0)

            searchModel.sourceModel = null
            searchModel.searchProperties = []
        }

        function test_d_multi_search() {
            repeater.model = null
            compare(repeater.count, 0)

            searchModel.pattern = ''
            searchModel.searchRoles = []
            searchModel.searchProperties = []
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
            searchModel.matchType = SearchModel.MatchBeginning

            searchModel.sourceModel = refineModel
            compare(searchModel.populated, true)
            compare(searchModel.count, 15)

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

            searchModel.pattern = 'a'
            compare(repeater.count, 10)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(9).orderValue, 10)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 15)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(14).orderValue, 15)

            searchModel.pattern = 'ab'
            compare(repeater.count, 12)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(11).orderValue, 15)

            searchModel.pattern = 'abcd'
            compare(repeater.count, 8)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(7).orderValue, 14)

            searchModel.pattern = 'abcdef'
            compare(repeater.count, 4)
            compare(repeater.itemAt(0).orderValue, 7)
            compare(repeater.itemAt(3).orderValue, 14)

            searchModel.pattern = 'abcde'
            compare(repeater.count, 7)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(6).orderValue, 14)

            searchModel.pattern = 'ab'
            compare(repeater.count, 12)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(11).orderValue, 15)

            searchModel.pattern = ''
            compare(repeater.count, 15)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(14).orderValue, 15)
        }

        function test_f_decomposition() {
            repeater.model = null
            compare(repeater.count, 0)

            searchModel.sourceModel = null
            searchModel.searchRoles = []
            searchModel.pattern = ''
            searchModel.matchType = SearchModel.MatchBeginning

            searchModel.sourceModel = decompositionModel
            compare(searchModel.populated, true)
            compare(searchModel.count, 8)

            repeater.model = searchModel

            searchModel.searchRoles = [ 'name' ]

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'Elvis'
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(2).orderValue, 4)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 4)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(3).orderValue, 4)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'elvis'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 2)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 4)
            compare(repeater.itemAt(0).orderValue, 1)
            compare(repeater.itemAt(3).orderValue, 4)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'Ëlvis'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 3)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 3)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'ëlvis'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 3)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'Elviß'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 4)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 4)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'elviß'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 4)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'Ælvis'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 5)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 5)
            compare(repeater.itemAt(1).orderValue, 6)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'ælvis'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 6)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 5)
            compare(repeater.itemAt(1).orderValue, 6)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'AElvis'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 5)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 5)
            compare(repeater.itemAt(1).orderValue, 6)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'aelvis'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 6)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 5)
            compare(repeater.itemAt(1).orderValue, 6)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'Aelvis'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 5)
            compare(repeater.itemAt(1).orderValue, 6)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'Alvis'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'alvis'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'Ølvis'
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 7)
            compare(repeater.itemAt(1).orderValue, 8)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 7)
            compare(repeater.itemAt(1).orderValue, 8)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'ølvis'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 7)
            compare(repeater.itemAt(1).orderValue, 8)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'Olvis'
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 7)
            compare(repeater.itemAt(1).orderValue, 8)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 7)
            compare(repeater.itemAt(1).orderValue, 8)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'olvis'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).orderValue, 7)
            compare(repeater.itemAt(1).orderValue, 8)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'Ølviß'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 8)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 8)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'ølviß'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 8)

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.pattern = 'olviss'
            compare(repeater.count, 0)

            searchModel.caseSensitivity = Qt.CaseInsensitive
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 8)
        }

        function test_g_apostrophe() {
            repeater.model = null
            compare(repeater.count, 0)

            searchModel.sourceModel = null
            searchModel.searchRoles = []
            searchModel.pattern = ''

            searchModel.sourceModel = apostropheModel
            compare(searchModel.populated, true)
            compare(searchModel.count, 2)

            repeater.model = searchModel

            searchModel.caseSensitivity = Qt.CaseSensitive
            searchModel.searchRoles = [ 'name' ]

            searchModel.pattern = 'D'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 1)

            searchModel.pattern = 'D\''
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 1)

            searchModel.pattern = 'D\'urbanville'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 1)

            searchModel.pattern = 'Talkin'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 2)

            searchModel.pattern = 'Talkin\''
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 2)

            searchModel.pattern = 'Talkin\' Blues'
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).orderValue, 2)
        }
    }
}
