#  Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
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
ADD_SUBDIRECTORY(translations)
FILE(GLOB DATABASE_ICONS icons/database/*.png)
INSTALL(FILES ${DATABASE_ICONS} DESTINATION ${DATA_INSTALL_DIR}/icons/database)
IF(UNIX AND NOT APPLE)
	INSTALL(DIRECTORY icons/application/ DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor
		FILES_MATCHING PATTERN "keepassx.png" PATTERN "keepassx.svgz")
	INSTALL(DIRECTORY icons/application/ DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor
		FILES_MATCHING PATTERN "application-x-keepassx.png" PATTERN "application-x-keepassx.svgz")
	INSTALL(FILES linux/keepassx.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
	INSTALL(FILES linux/keepassx.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages)
ENDIF(UNIX AND NOT APPLE)
IF(APPLE)
	INSTALL(FILES macosx/keepassx.icns DESTINATION ${DATA_INSTALL_DIR})
ENDIF()
INSTALL(DIRECTORY icons/application/ DESTINATION ${DATA_INSTALL_DIR}/icons/application
	FILES_MATCHING PATTERN "*.png" PATTERN "*.svgz")
ADD_CUSTOM_TARGET(icons
	COMMAND png2icns macosx/keepassx.icns
	icons/application/16x16/apps/keepassx.png
	icons/application/32x32/apps/keepassx.png
	icons/application/48x48/apps/keepassx.png
	icons/application/128x128/apps/keepassx.png
	icons/application/256x256/apps/keepassx.png
	COMMAND icotool -c -o windows/keepassx.ico
	icons/application/16x16/apps/keepassx.png
	icons/application/24x24/apps/keepassx.png
	icons/application/32x32/apps/keepassx.png
	icons/application/48x48/apps/keepassx.png
	icons/application/64x64/apps/keepassx.png
	icons/application/128x128/apps/keepassx.png
	icons/application/256x256/apps/keepassx.png
	VERBATIM
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
