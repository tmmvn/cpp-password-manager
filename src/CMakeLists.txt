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
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
CONFIGURE_FILE(config-keepassx.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-keepassx.h)
INCLUDE(GetGitRevisionDescription)
GET_GIT_HEAD_REVISION(GIT_REFSPEC GIT_HEAD)
GIT_DESCRIBE(GIT_DESCRIBE --long)
IF(NOT GIT_HEAD OR NOT GIT_DESCRIBE)
	SET(GIT_HEAD "")
	SET(GIT_DESCRIBE "")
ENDIF()
CONFIGURE_FILE(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)
SET(keepassx_SOURCES
	core/Config.cpp
	core/Database.cpp
	core/DatabaseIcons.cpp
	core/Endian.cpp
	core/Entry.cpp
	core/EntryAttachments.cpp
	core/EntryAttributes.cpp
	core/EntrySearcher.cpp
	core/FilePath.cpp
	core/Global.h
	core/Group.cpp
	core/InactivityTimer.cpp
	core/ListDeleter.h
	core/Metadata.cpp
	core/PasswordGenerator.cpp
	core/TimeDelta.cpp
	core/TimeInfo.cpp
	core/ToDbExporter.cpp
	core/Tools.cpp
	core/Translator.cpp
	core/UUID.cpp
	crypto/Crypto.cpp
	crypto/CryptoHash.cpp
	crypto/Random.cpp
	crypto/SymmetricCipher.cpp
	crypto/SymmetricCipherBackend.h
	crypto/SymmetricCipherGcrypt.cpp
	format/CsvExporter.cpp
	format/KeePass2.h
	format/KeePass2RandomStream.cpp
	format/KeePass2Reader.cpp
	format/KeePass2Repair.cpp
	format/KeePass2Writer.cpp
	format/KeePass2XmlReader.cpp
	format/KeePass2XmlWriter.cpp
	gui/AboutDialog.cpp
	gui/Application.cpp
	gui/ChangeMasterKeyWidget.cpp
	gui/Clipboard.cpp
	gui/DatabaseOpenWidget.cpp
	gui/DatabaseRepairWidget.cpp
	gui/DatabaseSettingsWidget.cpp
	gui/DatabaseTabWidget.cpp
	gui/DatabaseWidget.cpp
	gui/DatabaseWidgetStateSync.cpp
	gui/DialogWidget.cpp
	gui/DragTabBar.cpp
	gui/EditWidget.cpp
	gui/EditWidgetIcons.cpp
	gui/EditWidgetProperties.cpp
	gui/FileDialog.cpp
	gui/IconModels.cpp
	gui/LineEdit.cpp
	gui/MainWindow.cpp
	gui/MessageBox.cpp
	gui/PasswordEdit.cpp
	gui/PasswordGeneratorWidget.cpp
	gui/PasswordComboBox.cpp
	gui/SettingsWidget.cpp
	gui/SortFilterHideProxyModel.cpp
	gui/UnlockDatabaseWidget.cpp
	gui/WelcomeWidget.cpp
	gui/entry/EditEntryWidget.cpp
	gui/entry/EditEntryWidget_p.h
	gui/entry/EntryAttachmentsModel.cpp
	gui/entry/EntryAttributesModel.cpp
	gui/entry/EntryHistoryModel.cpp
	gui/entry/EntryModel.cpp
	gui/entry/EntryView.cpp
	gui/group/EditGroupWidget.cpp
	gui/group/GroupModel.cpp
	gui/group/GroupView.cpp
	keys/CompositeKey.cpp
	keys/CompositeKey_p.h
	keys/FileKey.cpp
	keys/Key.h
	keys/PasswordKey.cpp
	streams/HashedBlockStream.cpp
	streams/LayeredStream.cpp
	streams/qtiocompressor.cpp
	streams/StoreDataStream.cpp
	streams/SymmetricCipherStream.cpp
)
SET(keepassx_SOURCES_MAINEXE
	main.cpp
)
SET(keepassx_FORMS
	gui/AboutDialog.ui
	gui/ChangeMasterKeyWidget.ui
	gui/DatabaseOpenWidget.ui
	gui/DatabaseSettingsWidget.ui
	gui/EditWidget.ui
	gui/EditWidgetIcons.ui
	gui/EditWidgetProperties.ui
	gui/MainWindow.ui
	gui/PasswordGeneratorWidget.ui
	gui/SearchWidget.ui
	gui/SettingsWidgetGeneral.ui
	gui/SettingsWidgetSecurity.ui
	gui/WelcomeWidget.ui
	gui/entry/EditEntryWidgetAdvanced.ui
	gui/entry/EditEntryWidgetHistory.ui
	gui/entry/EditEntryWidgetMain.ui
	gui/group/EditGroupWidgetMain.ui
)
IF(MINGW)
	SET(keepassx_SOURCES_MAINEXE
		${keepassx_SOURCES_MAINEXE}
		${CMAKE_SOURCE_DIR}/share/windows/icon.rc)
ENDIF()
QT6_WRAP_UI(keepassx_SOURCES ${keepassx_FORMS})
ADD_LIBRARY(keepassx_core STATIC ${keepassx_SOURCES})
SET_TARGET_PROPERTIES(keepassx_core PROPERTIES COMPILE_DEFINITIONS KEEPASSX_BUILDING_CORE)
TARGET_LINK_LIBRARIES(keepassx_core Qt6::Core Qt6::Concurrent Qt6::Widgets)
ADD_EXECUTABLE(${PROGNAME} WIN32 MACOSX_BUNDLE ${keepassx_SOURCES_MAINEXE})
TARGET_LINK_LIBRARIES(${PROGNAME}
	keepassx_core
	Qt6::Core
	Qt6::Concurrent
	Qt6::Widgets
	${GCRYPT_LIBRARIES}
	${ZLIB_LIBRARIES})
SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES ENABLE_EXPORTS ON)
IF(APPLE)
	CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/share/macosx/Info.plist.cmake ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
	SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
ENDIF()
INSTALL(TARGETS ${PROGNAME}
	BUNDLE DESTINATION . COMPONENT Runtime
	RUNTIME DESTINATION ${BIN_INSTALL_DIR} COMPONENT Runtime)
IF(APPLE)
	IF(QT_MAC_USE_COCOA AND EXISTS "${QT_LIBRARY_DIR}/Resources/qt_menu.nib")
		INSTALL(DIRECTORY "${QT_LIBRARY_DIR}/Resources/qt_menu.nib"
			DESTINATION "${DATA_INSTALL_DIR}")
	ENDIF()
	SET(CPACK_GENERATOR "DragNDrop")
	SET(CPACK_DMG_FORMAT "UDBZ")
	SET(CPACK_DMG_VOLUME_NAME "${PROGNAME}")
	SET(CPACK_SYSTEM_NAME "OSX")
	SET(CPACK_STRIP_FILES ON)
	SET(CPACK_PACKAGE_FILE_NAME "${PROGNAME}-${KEEPASSX_VERSION_NUM}")
	INCLUDE(CPack)
	INCLUDE(DeployQt4)
	INSTALL_QT4_EXECUTABLE(${PROGNAME}.app "qjpeg;qgif;qico;qtaccessiblewidgets")
ENDIF()
IF(MINGW)
	SET(CPACK_GENERATOR "ZIP")
	SET(CPACK_STRIP_FILES ON)
	SET(CPACK_PACKAGE_FILE_NAME "${PROGNAME}-${KEEPASSX_VERSION_NUM}")
	INCLUDE(CPack)
	INCLUDE(DeployQt4)
	INSTALL_QT4_EXECUTABLE(${PROGNAME}.exe "qjpeg;qgif;qico;qtaccessiblewidgets")
ENDIF()
