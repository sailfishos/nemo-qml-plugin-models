/*
 * Copyright (C) 2019 Open Mobile Platform LLC
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
            property string firstName
            property string lastName
            property string email
            property string organization
        }
    }

    property string searchPattern

    ObjectListModel {
        id: sourceModelOne
        objectName: 'sourceModelOne'
        automaticRoles: true

        Component.onCompleted: {
            appendItem(item.createObject(this, { firstName: 'Alice', lastName: 'Bradshaw', email: 'some@email.tld', organization: 'Toys Ltd' }))
            appendItem(item.createObject(this, { firstName: 'Daniel', lastName: 'Carter', email: 'dan@carter.tld', organization: 'Toys Ltd' }))
            appendItem(item.createObject(this, { firstName: 'Fergus', lastName: 'Greshum', email: 'bright@light.tld', organization: 'Lighting Co' }))
            appendItem(item.createObject(this, { firstName: 'Helmut', lastName: 'Innes', email: 'wrought@iron.tld', organization: 'Blacksmiths Ltd' }))
            appendItem(item.createObject(this, { firstName: 'Jason', lastName: 'Axton', email: 'jason@axton.tld', organization: 'Parachutes Pty' }))
        }
    }

    SearchModel {
        id: firstNameStartsWithModel
        objectName: "firstNameStartsWithModel"
        sourceModel: sourceModelOne
        caseSensitivity: Qt.CaseInsensitive
        matchType: SearchModel.MatchBeginning
        searchRoles: "firstName"
        pattern: searchPattern
    }

    SearchModel {
        id: lastNameStartsWithModel
        objectName: "lastNameStartsWithModel"
        sourceModel: sourceModelOne
        caseSensitivity: Qt.CaseInsensitive
        matchType: SearchModel.MatchBeginning
        searchRoles: "lastName"
        pattern: searchPattern
    }

    SearchModel {
        id: detailStartsWithModel
        objectName: "detailStartsWithModel"
        sourceModel: sourceModelOne
        caseSensitivity: Qt.CaseInsensitive
        matchType: SearchModel.MatchBeginning
        searchRoles: [ "organization", "email" ]
        pattern: searchPattern
    }

    SearchModel {
        id: nameContainsModel
        objectName: "nameContainsModel"
        sourceModel: sourceModelOne
        caseSensitivity: Qt.CaseInsensitive
        matchType: SearchModel.MatchAnywhere
        searchRoles: [ "firstName", "lastName" ]
        pattern: searchPattern
    }

    SearchModel {
        id: detailContainsModel
        objectName: "detailContainsModel"
        sourceModel: sourceModelOne
        caseSensitivity: Qt.CaseInsensitive
        matchType: SearchModel.MatchAnywhere
        searchRoles: [ "organization", "email" ]
        pattern: searchPattern
    }

    CombinedSearchModel {
        id: combinedModel
        models: [
            firstNameStartsWithModel,
            lastNameStartsWithModel,
            detailStartsWithModel,
            nameContainsModel,
            detailContainsModel
        ]
    }

    Repeater {
        id: repeater
        model: combinedModel
        delegate: Item {
            property string firstNameValue: firstName
            property string lastNameValue: lastName
            property string emailValue: email
            property string organizationValue: organization
            property string sourceModelValue: sourceModel
        }
    }

    resources: TestCase {
        name: "CombinedSearchModel"

        function init() {
        }
        function cleanup() {
        }

        function test_a_duplicates() {
            compare(repeater.count, 5)
            compare(repeater.itemAt(0).firstNameValue, 'Alice')
            compare(repeater.itemAt(0).lastNameValue, 'Bradshaw')
            compare(repeater.itemAt(0).emailValue, 'some@email.tld')
            compare(repeater.itemAt(0).organizationValue, 'Toys Ltd')
            compare(repeater.itemAt(0).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(1).firstNameValue, 'Daniel')
            compare(repeater.itemAt(1).lastNameValue, 'Carter')
            compare(repeater.itemAt(1).emailValue, 'dan@carter.tld')
            compare(repeater.itemAt(1).organizationValue, 'Toys Ltd')
            compare(repeater.itemAt(1).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(2).firstNameValue, 'Fergus')
            compare(repeater.itemAt(2).lastNameValue, 'Greshum')
            compare(repeater.itemAt(2).emailValue, 'bright@light.tld')
            compare(repeater.itemAt(2).organizationValue, 'Lighting Co')
            compare(repeater.itemAt(2).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(3).firstNameValue, 'Helmut')
            compare(repeater.itemAt(3).lastNameValue, 'Innes')
            compare(repeater.itemAt(3).emailValue, 'wrought@iron.tld')
            compare(repeater.itemAt(3).organizationValue, 'Blacksmiths Ltd')
            compare(repeater.itemAt(3).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(4).firstNameValue, 'Jason')
            compare(repeater.itemAt(4).lastNameValue, 'Axton')
            compare(repeater.itemAt(4).emailValue, 'jason@axton.tld')
            compare(repeater.itemAt(4).organizationValue, 'Parachutes Pty')
            compare(repeater.itemAt(4).sourceModelValue, 'sourceModelOne')

            searchPattern = "A"
            compare(repeater.count, 4) // Alice, Axton, dAniel, blAcksmiths.
            compare(repeater.itemAt(0).firstNameValue, 'Alice')
            compare(repeater.itemAt(0).lastNameValue, 'Bradshaw')
            compare(repeater.itemAt(0).emailValue, 'some@email.tld')
            compare(repeater.itemAt(0).organizationValue, 'Toys Ltd')
            compare(repeater.itemAt(0).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(1).firstNameValue, 'Jason')
            compare(repeater.itemAt(1).lastNameValue, 'Axton')
            compare(repeater.itemAt(1).emailValue, 'jason@axton.tld')
            compare(repeater.itemAt(1).organizationValue, 'Parachutes Pty')
            compare(repeater.itemAt(1).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(2).firstNameValue, 'Daniel')
            compare(repeater.itemAt(2).lastNameValue, 'Carter')
            compare(repeater.itemAt(2).emailValue, 'dan@carter.tld')
            compare(repeater.itemAt(2).organizationValue, 'Toys Ltd')
            compare(repeater.itemAt(2).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(3).firstNameValue, 'Helmut')
            compare(repeater.itemAt(3).lastNameValue, 'Innes')
            compare(repeater.itemAt(3).emailValue, 'wrought@iron.tld')
            compare(repeater.itemAt(3).organizationValue, 'Blacksmiths Ltd')
            compare(repeater.itemAt(3).sourceModelValue, 'sourceModelOne')

            searchPattern = "AX"
            compare(repeater.count, 1) // AXton
            compare(repeater.itemAt(0).firstNameValue, 'Jason')
            compare(repeater.itemAt(0).lastNameValue, 'Axton')
            compare(repeater.itemAt(0).emailValue, 'jason@axton.tld')
            compare(repeater.itemAt(0).organizationValue, 'Parachutes Pty')
            compare(repeater.itemAt(0).sourceModelValue, 'sourceModelOne')

            searchPattern = "G"
            compare(repeater.count, 2) // Greshum, wrouGht@iron.tld
            compare(repeater.itemAt(0).firstNameValue, 'Fergus')
            compare(repeater.itemAt(0).lastNameValue, 'Greshum')
            compare(repeater.itemAt(0).emailValue, 'bright@light.tld')
            compare(repeater.itemAt(0).organizationValue, 'Lighting Co')
            compare(repeater.itemAt(0).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(1).firstNameValue, 'Helmut')
            compare(repeater.itemAt(1).lastNameValue, 'Innes')
            compare(repeater.itemAt(1).emailValue, 'wrought@iron.tld')
            compare(repeater.itemAt(1).organizationValue, 'Blacksmiths Ltd')
            compare(repeater.itemAt(1).sourceModelValue, 'sourceModelOne')

            sourceModelOne.appendItem(item.createObject(this, { firstName: 'Gary', lastName: 'Kelly', email: 'kelly@light.tld', organization: 'Lighting Co' }))
            compare(repeater.count, 3) // Gary, Greshum, wrouGht@iron.tld
            compare(repeater.itemAt(0).firstNameValue, 'Gary')
            compare(repeater.itemAt(0).lastNameValue, 'Kelly')
            compare(repeater.itemAt(0).emailValue, 'kelly@light.tld')
            compare(repeater.itemAt(0).organizationValue, 'Lighting Co')
            compare(repeater.itemAt(0).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(1).firstNameValue, 'Fergus')
            compare(repeater.itemAt(1).lastNameValue, 'Greshum')
            compare(repeater.itemAt(1).emailValue, 'bright@light.tld')
            compare(repeater.itemAt(1).organizationValue, 'Lighting Co')
            compare(repeater.itemAt(1).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(2).firstNameValue, 'Helmut')
            compare(repeater.itemAt(2).lastNameValue, 'Innes')
            compare(repeater.itemAt(2).emailValue, 'wrought@iron.tld')
            compare(repeater.itemAt(2).organizationValue, 'Blacksmiths Ltd')
            compare(repeater.itemAt(2).sourceModelValue, 'sourceModelOne')

            sourceModelOne.appendItem(item.createObject(this, { firstName: 'Mark', lastName: 'Kelly', email: 'mark@kelly.tld', organization: 'Gimbals Ltd' }))
            compare(repeater.count, 4) // Gary, Greshum, Gimbals, wrouGht@iron.tld
            compare(repeater.itemAt(0).firstNameValue, 'Gary')
            compare(repeater.itemAt(0).lastNameValue, 'Kelly')
            compare(repeater.itemAt(0).emailValue, 'kelly@light.tld')
            compare(repeater.itemAt(0).organizationValue, 'Lighting Co')
            compare(repeater.itemAt(0).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(1).firstNameValue, 'Fergus')
            compare(repeater.itemAt(1).lastNameValue, 'Greshum')
            compare(repeater.itemAt(1).emailValue, 'bright@light.tld')
            compare(repeater.itemAt(1).organizationValue, 'Lighting Co')
            compare(repeater.itemAt(1).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(2).firstNameValue, 'Mark')
            compare(repeater.itemAt(2).lastNameValue, 'Kelly')
            compare(repeater.itemAt(2).emailValue, 'mark@kelly.tld')
            compare(repeater.itemAt(2).organizationValue, 'Gimbals Ltd')
            compare(repeater.itemAt(2).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(3).firstNameValue, 'Helmut')
            compare(repeater.itemAt(3).lastNameValue, 'Innes')
            compare(repeater.itemAt(3).emailValue, 'wrought@iron.tld')
            compare(repeater.itemAt(3).organizationValue, 'Blacksmiths Ltd')
            compare(repeater.itemAt(3).sourceModelValue, 'sourceModelOne')

            sourceModelOne.removeItemAt(2) // remove Fergus Greshum
            compare(repeater.count, 3) // Gary, Gimbals, wrouGht@iron.tld
            compare(repeater.itemAt(0).firstNameValue, 'Gary')
            compare(repeater.itemAt(0).lastNameValue, 'Kelly')
            compare(repeater.itemAt(0).emailValue, 'kelly@light.tld')
            compare(repeater.itemAt(0).organizationValue, 'Lighting Co')
            compare(repeater.itemAt(0).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(1).firstNameValue, 'Mark')
            compare(repeater.itemAt(1).lastNameValue, 'Kelly')
            compare(repeater.itemAt(1).emailValue, 'mark@kelly.tld')
            compare(repeater.itemAt(1).organizationValue, 'Gimbals Ltd')
            compare(repeater.itemAt(1).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(2).firstNameValue, 'Helmut')
            compare(repeater.itemAt(2).lastNameValue, 'Innes')
            compare(repeater.itemAt(2).emailValue, 'wrought@iron.tld')
            compare(repeater.itemAt(2).organizationValue, 'Blacksmiths Ltd')
            compare(repeater.itemAt(2).sourceModelValue, 'sourceModelOne')

            searchPattern = ""
            compare(repeater.count, 6)
            compare(repeater.itemAt(0).firstNameValue, 'Alice')
            compare(repeater.itemAt(0).lastNameValue, 'Bradshaw')
            compare(repeater.itemAt(0).emailValue, 'some@email.tld')
            compare(repeater.itemAt(0).organizationValue, 'Toys Ltd')
            compare(repeater.itemAt(0).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(1).firstNameValue, 'Daniel')
            compare(repeater.itemAt(1).lastNameValue, 'Carter')
            compare(repeater.itemAt(1).emailValue, 'dan@carter.tld')
            compare(repeater.itemAt(1).organizationValue, 'Toys Ltd')
            compare(repeater.itemAt(1).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(2).firstNameValue, 'Helmut')
            compare(repeater.itemAt(2).lastNameValue, 'Innes')
            compare(repeater.itemAt(2).emailValue, 'wrought@iron.tld')
            compare(repeater.itemAt(2).organizationValue, 'Blacksmiths Ltd')
            compare(repeater.itemAt(2).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(3).firstNameValue, 'Jason')
            compare(repeater.itemAt(3).lastNameValue, 'Axton')
            compare(repeater.itemAt(3).emailValue, 'jason@axton.tld')
            compare(repeater.itemAt(3).organizationValue, 'Parachutes Pty')
            compare(repeater.itemAt(3).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(4).firstNameValue, 'Gary')
            compare(repeater.itemAt(4).lastNameValue, 'Kelly')
            compare(repeater.itemAt(4).emailValue, 'kelly@light.tld')
            compare(repeater.itemAt(4).organizationValue, 'Lighting Co')
            compare(repeater.itemAt(4).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(5).firstNameValue, 'Mark')
            compare(repeater.itemAt(5).lastNameValue, 'Kelly')
            compare(repeater.itemAt(5).emailValue, 'mark@kelly.tld')
            compare(repeater.itemAt(5).organizationValue, 'Gimbals Ltd')
            compare(repeater.itemAt(5).sourceModelValue, 'sourceModelOne')

            searchPattern = "G"
            compare(repeater.count, 3) // Gary, Gimbals, wrouGht@iron.tld
            compare(repeater.itemAt(0).firstNameValue, 'Gary')
            compare(repeater.itemAt(0).lastNameValue, 'Kelly')
            compare(repeater.itemAt(0).emailValue, 'kelly@light.tld')
            compare(repeater.itemAt(0).organizationValue, 'Lighting Co')
            compare(repeater.itemAt(0).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(1).firstNameValue, 'Mark')
            compare(repeater.itemAt(1).lastNameValue, 'Kelly')
            compare(repeater.itemAt(1).emailValue, 'mark@kelly.tld')
            compare(repeater.itemAt(1).organizationValue, 'Gimbals Ltd')
            compare(repeater.itemAt(1).sourceModelValue, 'sourceModelOne')
            compare(repeater.itemAt(2).firstNameValue, 'Helmut')
            compare(repeater.itemAt(2).lastNameValue, 'Innes')
            compare(repeater.itemAt(2).emailValue, 'wrought@iron.tld')
            compare(repeater.itemAt(2).organizationValue, 'Blacksmiths Ltd')
            compare(repeater.itemAt(2).sourceModelValue, 'sourceModelOne')
        }
    }
}
