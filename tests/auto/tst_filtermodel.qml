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
                name: 'Bob',
                gender: 'male',
                pets: []
            })
            append({
                order: 3,
                name: 'Charlie',
                gender: 'male',
                pets: [ 'dog', 'hamster' ]
            })
            append({
                order: 4,
                name: 'Debbie',
                gender: 'female',
                pets: [ 'cat' ]
            })
            append({
                order: 5,
                name: 'Eddie',
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

    FilterModel {
        id: filterModel
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
            compare(filterModel.populated, false)
            compare(filterModel.count, 0)
        }

        function test_b_unfiltered() {
            filterModel.sourceModel = baseModel
            compare(filterModel.populated, true)
            compare(filterModel.count, 5)
            compare(repeater.count, 0)

            repeater.model = filterModel
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(2).orderValue, 3)
            compare(repeater.itemAt(4).nameValue, 'Eddie')

            repeater.model = null
            compare(repeater.count, 0)

            filterModel.sourceModel = null
            compare(filterModel.populated, false)
            compare(filterModel.count, 0)

            repeater.model = filterModel
            compare(repeater.count, 0)

            filterModel.sourceModel = otherModel
            compare(filterModel.populated, true)
            compare(filterModel.count, 3)
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).categoryValue, 'soldier')
            compare(repeater.itemAt(1).orderText, 'guard')
            compare(repeater.itemAt(2).nameValue, '')

            repeater.model = undefined
            compare(repeater.count, 0)
        }

        function test_c_filtered() {
            repeater.model = null
            compare(repeater.count, 0)

            filterModel.sourceModel = baseModel
            compare(filterModel.populated, true)
            compare(filterModel.count, 5)

            repeater.model = filterModel
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(4).nameValue, 'Eddie')

            filterModel.filters = [{ 'role': 'gender', 'comparator': '==', 'value': 'male' }]
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Bob')
            compare(repeater.itemAt(2).nameValue, 'Eddie')

            filterModel.filters = [{ 'role': 'gender', 'comparator': '==', 'value': 'female' }]
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(1).nameValue, 'Debbie')

            filterModel.filters = [{ 'role': 'gender', 'comparator': '!=', 'value': 'male' }]
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(1).nameValue, 'Debbie')

            filterModel.filters = [{ 'role': 'order', 'comparator': '<', 'value': 3 }]
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(1).nameValue, 'Bob')

            filterModel.filters = [{ 'role': 'order', 'comparator': '<=', 'value': 3 }]
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(2).nameValue, 'Charlie')

            filterModel.filters = [{ 'role': 'order', 'comparator': '>', 'value': 3 }]
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).nameValue, 'Debbie')
            compare(repeater.itemAt(1).nameValue, 'Eddie')

            filterModel.filters = [{ 'role': 'order', 'comparator': '>=', 'value': 3 }]
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Charlie')
            compare(repeater.itemAt(2).nameValue, 'Eddie')

            filterModel.filters = [{ 'role': 'name', 'comparator': 'match', 'value': 'ie\\b' }]
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Charlie')
            compare(repeater.itemAt(2).nameValue, 'Eddie')

            filterModel.filters = [{ 'role': 'name', 'comparator': '!match', 'value': 'ie\\b' }]
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(1).nameValue, 'Bob')

            /* TODO: Does not work with ListElement members...
            filterModel.filters = [{ 'role': 'pets', 'comparator': 'contains', 'value': 'cat' }]
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(2).nameValue, 'Eddie')

            filterModel.filters = [{ 'role': 'pets', 'comparator': 'contains', 'value': 'dog' }]
            compare(repeater.count, 3)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(2).nameValue, 'Charlie')

            filterModel.filters = [{ 'role': 'pets', 'comparator': '!contains', 'value': 'goldfish' }]
            compare(repeater.count, 4)
            compare(repeater.itemAt(0).nameValue, 'Bob')
            compare(repeater.itemAt(3).nameValue, 'Eddie')
            */

            filterModel.filters = []
            compare(repeater.count, 5)
        }

        function test_d_multi_filtered() {
            repeater.model = null
            compare(repeater.count, 0)

            filterModel.sourceModel = baseModel
            compare(filterModel.populated, true)
            compare(filterModel.count, 5)
            compare(filterModel.filterRequirement, FilterModel.PassAllFilters)

            repeater.model = filterModel
            compare(repeater.count, 5)

            filterModel.filters = [{ 'role': 'gender', 'comparator': '==', 'value': 'male' }, { 'role': 'order', 'comparator': '>=', 'value': 3 }]
            compare(repeater.count, 2)
            compare(repeater.itemAt(0).nameValue, 'Charlie')
            compare(repeater.itemAt(1).nameValue, 'Eddie')

            filterModel.filterRequirement = FilterModel.PassAnyFilter
            compare(repeater.count, 4)
            compare(repeater.itemAt(0).nameValue, 'Bob')
            compare(repeater.itemAt(3).nameValue, 'Eddie')

            filterModel.filterRequirement = FilterModel.PassAllFilters
            compare(repeater.count, 2)

            filterModel.filters = [{ 'role': 'gender', 'comparator': '==', 'value': 'female' }, { 'role': 'name', 'comparator': 'match', 'value': 'ie\\b' }]
            compare(repeater.count, 1)
            compare(repeater.itemAt(0).nameValue, 'Debbie')

            filterModel.filterRequirement = FilterModel.PassAnyFilter
            compare(repeater.count, 4)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(3).nameValue, 'Eddie')

            filterModel.filterRequirement = FilterModel.PassAllFilters
            compare(repeater.count, 1)

            filterModel.filters = [{ 'role': 'order', 'comparator': '>', 'value': 3 }, { 'role': 'order', 'comparator': '<', 'value': 3 }]
            compare(repeater.count, 0)

            filterModel.filterRequirement = FilterModel.PassAnyFilter
            compare(repeater.count, 4)
            compare(repeater.itemAt(0).nameValue, 'Alice')
            compare(repeater.itemAt(4).nameValue, 'Eddie')

            filterModel.filterRequirement = FilterModel.PassAllFilters
            compare(repeater.count, 0)

            filterModel.filters = []
            compare(repeater.count, 5)
        }
    }
}
