#  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 or (at your option)
#  version 3 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src)
ADD_DEFINITIONS(-DQT_TEST_LIB)
SET(KEEPASSX_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)
CONFIGURE_FILE(config-keepassx-tests.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-keepassx-tests.h)
MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
	SET(DEFAULT_ARGS)
	FOREACH(arg_name ${arg_names})
		SET(${prefix}_${arg_name})
	ENDFOREACH(arg_name)
	FOREACH(option ${option_names})
		SET(${prefix}_${option} FALSE)
	ENDFOREACH(option)
	SET(current_arg_name DEFAULT_ARGS)
	SET(current_arg_list)
	FOREACH(arg ${ARGN})
		SET(larg_names ${arg_names})
		LIST(FIND larg_names "${arg}" is_arg_name)
		IF(is_arg_name GREATER -1)
			SET(${prefix}_${current_arg_name} ${current_arg_list})
			SET(current_arg_name ${arg})
			SET(current_arg_list)
		ELSE()
			SET(loption_names ${option_names})
			LIST(FIND loption_names "${arg}" is_option)
			IF(is_option GREATER -1)
				SET(${prefix}_${arg} TRUE)
			ELSE(is_option GREATER -1)
				SET(current_arg_list ${current_arg_list} ${arg})
			ENDIF()
		ENDIF()
	ENDFOREACH(arg)
	SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)
MACRO(ADD_UNIT_TEST)
	PARSE_ARGUMENTS(TEST "NAME;SOURCES;LIBS" "" ${ARGN})
	SET(_test_NAME ${TEST_NAME})
	SET(_srcList ${TEST_SOURCES})
	ADD_EXECUTABLE(${_test_NAME} ${_srcList})
	TARGET_LINK_LIBRARIES(${_test_NAME} ${TEST_LIBS})
	IF(NOT TEST_OUTPUT)
		SET(TEST_OUTPUT plaintext)
	ENDIF(NOT TEST_OUTPUT)
	SET(TEST_OUTPUT ${TEST_OUTPUT} CACHE STRING "The output to generate when running the QTest unit tests")
	IF(KDE4_TEST_OUTPUT STREQUAL "xml")
		ADD_TEST(${_test_NAME} ${_test_NAME} -xml -o ${_test_NAME}.tml)
	ELSE(KDE4_TEST_OUTPUT STREQUAL "xml")
		ADD_TEST(${_test_NAME} ${_test_NAME})
	ENDIF(KDE4_TEST_OUTPUT STREQUAL "xml")
	IF(NOT MSVC_IDE)   #not needed for the ide
		# if the tests are EXCLUDE_FROM_ALL, add a target "buildtests" to build all tests
		IF(NOT WITH_TESTS)
			GET_DIRECTORY_PROPERTY(_buildtestsAdded BUILDTESTS_ADDED)
			IF(NOT _buildtestsAdded)
				ADD_CUSTOM_TARGET(buildtests)
				SET_DIRECTORY_PROPERTIES(PROPERTIES BUILDTESTS_ADDED TRUE)
			ENDIF()
			ADD_DEPENDENCIES(buildtests ${_test_NAME})
		ENDIF()
	ENDIF()
ENDMACRO(ADD_UNIT_TEST)
SET(TEST_LIBRARIES
	keepassx_core
	Qt6::Core
	Qt6::Concurrent
	Qt6::Widgets
	Qt6::Test
	${GCRYPT_LIBRARIES}
	${ZLIB_LIBRARIES}
)
SET(testsupport_SOURCES modeltest.cpp FailDevice.cpp)
ADD_LIBRARY(testsupport STATIC ${testsupport_SOURCES})
TARGET_LINK_LIBRARIES(testsupport Qt6::Core Qt6::Concurrent Qt6::Widgets
	Qt6::Test)
ADD_UNIT_TEST(NAME testgroup SOURCES TestGroup.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testkeepass2xmlreader SOURCES TestKeePass2XmlReader.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testkeys SOURCES TestKeys.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testkeepass2reader SOURCES TestKeePass2Reader.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testkeepass2writer SOURCES TestKeePass2Writer.cpp
	LIBS testsupport ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testgroupmodel SOURCES TestGroupModel.cpp
	LIBS testsupport ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testentrymodel SOURCES TestEntryModel.cpp
	LIBS testsupport ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testcryptohash SOURCES TestCryptoHash.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testsymmetriccipher SOURCES TestSymmetricCipher.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testhashedblockstream SOURCES TestHashedBlockStream.cpp
	LIBS testsupport ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testkeepass2randomstream SOURCES TestKeePass2RandomStream.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testmodified SOURCES TestModified.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testdeletedobjects SOURCES TestDeletedObjects.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testentry SOURCES TestEntry.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testrandom SOURCES TestRandom.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testentrysearcher SOURCES TestEntrySearcher.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testexporter SOURCES TestExporter.cpp
	LIBS ${TEST_LIBRARIES})
ADD_UNIT_TEST(NAME testcsvexporter SOURCES TestCsvExporter.cpp
	LIBS ${TEST_LIBRARIES})
IF(WITH_GUI_TESTS)
	ADD_SUBDIRECTORY(gui)
ENDIF(WITH_GUI_TESTS)
