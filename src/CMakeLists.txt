
SET(glues_lib_SRCS
        external/glues_stel/source/glues_error.c
        external/glues_stel/source/glues_error.h
        external/glues_stel/source/glues.h
        external/glues_stel/source/glues_mipmap.c
        external/glues_stel/source/glues_mipmap.h
        external/glues_stel/source/libtess/dict.h
        external/glues_stel/source/libtess/dict.c
        external/glues_stel/source/libtess/dict-list.h
        external/glues_stel/source/libtess/geom.c
        external/glues_stel/source/libtess/geom.h
        external/glues_stel/source/libtess/memalloc.c
        external/glues_stel/source/libtess/memalloc.h
        external/glues_stel/source/libtess/mesh.c
        external/glues_stel/source/libtess/mesh.h
        external/glues_stel/source/libtess/normal.c
        external/glues_stel/source/libtess/normal.h
        external/glues_stel/source/libtess/priorityq.c
        external/glues_stel/source/libtess/priorityq.h
        external/glues_stel/source/libtess/priorityq-heap.h
        external/glues_stel/source/libtess/priorityq-heap.i
        external/glues_stel/source/libtess/priorityq-sort.h
        external/glues_stel/source/libtess/render.c
        external/glues_stel/source/libtess/render.h
        external/glues_stel/source/libtess/sweep.c
        external/glues_stel/source/libtess/sweep.h
        external/glues_stel/source/libtess/tess.c
        external/glues_stel/source/libtess/tess.h
        external/glues_stel/source/libtess/tessmono.c
        external/glues_stel/source/libtess/tessmono.h
        )


IF (OPENGL_MODE STREQUAL "Desktop")
  SET(gl_lib_SRCS external/GLee.c external/GLee.h)
ENDIF()
IF(OPENGL_MODE STREQUAL "ES2")
  SET(gl_lib_SRCS external/GLee.h)
ENDIF()

SET(shiraplayer_lib_SRCS
        #core/StelAudioMgr.hpp
        #core/StelAudioMgr.cpp
        core/StelGeodesicGrid.cpp
        core/StelGeodesicGrid.hpp
        core/StelLoadingBar.cpp
        core/StelLoadingBar.hpp
        core/StelMovementMgr.cpp
        core/StelMovementMgr.hpp
        core/StelNavigator.cpp
        core/StelNavigator.hpp
        core/StelObserver.cpp
        core/StelObserver.hpp
        core/StelLocation.hpp
        core/StelLocation.cpp
        core/StelLocationMgr.hpp
        core/StelLocationMgr.cpp
        core/StelProjector.cpp
        core/StelProjector.hpp
        core/StelProjectorClasses.cpp
        core/StelProjectorClasses.hpp
        core/StelProjectorType.hpp
        core/StelSkyDrawer.cpp
        core/StelSkyDrawer.hpp
        core/StelPainter.hpp
        core/StelPainter.cpp
        core/MultiLevelJsonBase.hpp
        core/MultiLevelJsonBase.cpp
        core/StelSkyImageTile.hpp
        core/StelSkyImageTile.cpp

        core/StelPresentImageTile.hpp
        core/StelPresentImageTile.cpp

        core/StelSkyPolygon.hpp
        core/StelSkyPolygon.cpp
        core/SphericMirrorCalculator.cpp
        core/SphericMirrorCalculator.hpp
        core/StelApp.cpp
        core/StelApp.hpp
        core/StelCore.cpp
        core/StelCore.hpp
        core/StelFileMgr.cpp
        core/StelFileMgr.hpp
        core/StelAppGraphicsWidget.cpp
        core/StelAppGraphicsWidget.hpp
        core/StelLocaleMgr.cpp
        core/StelLocaleMgr.hpp
        core/StelModule.cpp
        core/StelModule.hpp
        core/StelModuleMgr.cpp
        core/StelModuleMgr.hpp
        core/StelObject.cpp
        core/StelObject.hpp
        core/StelObjectMgr.cpp
        core/StelObjectMgr.hpp
        core/StelObjectModule.cpp
        core/StelObjectModule.hpp
        core/StelObjectType.hpp
        core/StelOpenGL.hpp
        core/StelOpenGL.cpp
        core/StelPluginInterface.hpp
        core/StelSkyCultureMgr.cpp
        core/StelSkyCultureMgr.hpp
        core/StelTextureMgr.cpp
        core/StelTextureMgr.hpp
        core/StelTexture.cpp
        core/StelTexture.hpp
        core/StelTextureTypes.hpp
        core/StelToneReproducer.cpp
        core/StelToneReproducer.hpp
        core/StelStyle.hpp
        core/StelSkyLayerMgr.cpp
        core/StelSkyLayerMgr.hpp

        core/StelPresentMgr.hpp
        core/StelPresentMgr.cpp

        core/StelSkyLayer.hpp
        core/StelSkyLayer.cpp
        core/StelFader.hpp
        core/StelSphereGeometry.cpp
        core/StelSphereGeometry.hpp
        core/OctahedronPolygon.cpp
        core/OctahedronPolygon.hpp
        core/StelIniParser.cpp
        core/StelIniParser.hpp
        core/StelUtils.cpp
        core/StelUtils.hpp
        core/StelTranslator.cpp
        core/StelTranslator.hpp
        core/VecMath.hpp
        core/StelJsonParser.hpp
        core/StelJsonParser.cpp
        core/SimbadSearcher.hpp
        core/SimbadSearcher.cpp
        core/StelSphericalIndex.hpp
        core/StelSphericalIndex.cpp
        core/StelVertexArray.hpp
        core/StelVertexArray.cpp
        core/StelGuiBase.hpp
        core/StelGuiBase.cpp

        core/StelShader.hpp
        core/StelShader.cpp

        #core/StelViewportEffect.hpp
        #core/StelViewportEffect.cpp
        core/StelViewportDistorter.hpp
        core/StelViewportDistorter.cpp
        core/TrailGroup.hpp
        core/TrailGroup.cpp
        core/RefractionExtinction.hpp
        core/RefractionExtinction.cpp
        ${gl_lib_SRCS}
        external/fixx11h.h
        external/gettext.h

        external/kfilter/kfilterdev.h
        external/kfilter/kfilterdev.cpp
        external/kfilter/kfilterbase.h
        external/kfilter/kfilterbase.cpp
        external/kfilter/kgzipfilter.h
        external/kfilter/kgzipfilter.cpp
        external/kfilter/kzip.h
        external/kfilter/kzip.cpp
        external/kfilter/klimitediodevice.h
        external/kfilter/klimitediodevice.cpp
        external/kfilter/karchive.h
        external/kfilter/karchive.cpp

        ${glues_lib_SRCS}

        planetsephems/calc_interpolated_elements.c
        planetsephems/calc_interpolated_elements.h
        planetsephems/elliptic_to_rectangular.c
        planetsephems/elliptic_to_rectangular.h
        planetsephems/elp82b.c
        planetsephems/elp82b.h
        planetsephems/gust86.c
        planetsephems/gust86.h
        planetsephems/l1.c
        planetsephems/l1.h
        planetsephems/marssat.c
        planetsephems/marssat.h
        planetsephems/misc_stellplanet.c
        planetsephems/misc_stellplanet.h
        planetsephems/pluto.c
        planetsephems/pluto.h
        planetsephems/sideral_time.c
        planetsephems/sideral_time.h
        planetsephems/stellplanet.c
        planetsephems/stellplanet.h
        planetsephems/tass17.c
        planetsephems/tass17.h
        planetsephems/vsop87.c
        planetsephems/vsop87.h

        modules/Atmosphere.cpp
        modules/Atmosphere.hpp
        modules/Constellation.cpp
        modules/Constellation.hpp
        modules/ConstellationMgr.cpp
        modules/ConstellationMgr.hpp
        modules/GridLinesMgr.cpp
        modules/GridLinesMgr.hpp
        modules/LabelMgr.hpp
        modules/LabelMgr.cpp
        modules/Landscape.cpp
        modules/Landscape.hpp
        modules/LandscapeMgr.cpp
        modules/LandscapeMgr.hpp
        modules/Meteor.cpp
        modules/Meteor.hpp
        modules/MeteorMgr.cpp
        modules/MeteorMgr.hpp
        modules/MilkyWay.cpp
        modules/MilkyWay.hpp
        modules/Nebula.cpp
        modules/Nebula.hpp
        modules/NebulaMgr.cpp
        modules/NebulaMgr.hpp
        modules/Orbit.cpp
        modules/Orbit.hpp
        modules/Planet.cpp
        modules/Planet.hpp
        modules/ScreenImageMgr.hpp
        modules/ScreenImageMgr.cpp
        modules/Skybright.cpp
        modules/Skybright.hpp
        modules/Skylight.cpp
        modules/Skylight.hpp
        modules/SolarSystem.cpp
        modules/SolarSystem.hpp
        modules/Solve.hpp
        modules/Star.cpp
        modules/Star.hpp
        modules/StarMgr.cpp
        modules/StarMgr.hpp
        modules/StarWrapper.cpp
        modules/StarWrapper.hpp
        modules/Telescope.cpp
        modules/Telescope.hpp
        modules/TelescopeMgr.cpp
        modules/TelescopeMgr.hpp
        modules/ZoneArray.cpp
        modules/ZoneArray.hpp
        modules/ZoneData.hpp

        scripting/StelScriptMgr.cpp
        scripting/StratoscriptPreprocessor.cpp
        scripting/StelScriptMgr.hpp
        scripting/ScriptSleeper.cpp
        scripting/ScriptSleeper.hpp
        scripting/StelMainScriptAPI.cpp
        scripting/StelMainScriptAPI.hpp
        scripting/StelMainScriptAPIProxy.cpp
        scripting/StelMainScriptAPIProxy.hpp

        #socketutils/mysocket.h
        #socketutils/mysocket.cpp

        socketutils/rsync.h
        socketutils/rsync.cpp

        shiraplayerform.hpp
        shiraplayerform.cpp
        StelMainGraphicsView.hpp
        StelMainGraphicsView.cpp
        stelqglwidget.h
        stelqglwidget.cpp

        StelMainWindow.hpp
        StelMainWindow.cpp
        StelLogger.hpp
        StelLogger.cpp
        CLIProcessor.hpp
        CLIProcessor.cpp
        translations.h

        core/SFont.cpp
        core/SFont.hpp
        core/typeface.cpp
        core/typeface.h
        core/VecMath.hpp
        core/StelFontMgr.cpp
        core/StelFontMgr.hpp

        warping/Warp.h
        warping/Warp.cpp
        warping/channel.h
        warping/channel.cpp
        modules/networkcommands.h
        modules/networkcommands.cpp

        socketutils/sntpclient.h
        socketutils/sntpclient.cpp

        videoutils/videoclass.h
        videoutils/videoclass.cpp
        videoutils/audioclass.h
        videoutils/audioclass.cpp
        videoutils/worker.h
        videoutils/worker.cpp
        videoutils/embedaudiowarning.h
        videoutils/embedaudiowarning.cpp


        encrypt/cBinExt.cpp
        encrypt/cBinExt.h
        encrypt/hash_defines.h

        blowfish/blowfish.h
        blowfish/blowfish.c

        recordutils/QVideoEncoder.h
        recordutils/QVideoEncoder.cpp

        recordutils/glthread.h
        recordutils/glthread.cpp

        licenceutils/qlisansform.h
        licenceutils/qlisansform.cpp

        presenter/qplayimagewidget.h
        presenter/qplayimagewidget.cpp
        presenter/qimagepropwidget.h
        presenter/qimagepropwidget.cpp
        presenter/qmediabrowserwidget.h
        presenter/qmediabrowserwidget.cpp
        presenter/VideoSource.h
        presenter/VideoSource.cpp
        presenter/flowlayout.h
        presenter/flowlayout.cpp

        previewutil/previewform.hpp
        previewutil/previewform.cpp

        freehand/tupgraphicalgorithm.h
        freehand/tupgraphicalgorithm.cpp
        freehand/freehandItem.hpp
        freehand/freehandItem.cpp
        freehand/tupcolorpalette.h
        freehand/tupcolorpalette.cpp
        freehand/tupslider.h
        freehand/tupslider.cpp
        freehand/tupcolorwidget.h
        freehand/tupcolorwidget.cpp
        freehand/tuppenpreviewcanvas.h
        freehand/tuppenpreviewcanvas.cpp

        flybyutil/flybymanager.h
        flybyutil/flybymanager.cpp

        #glrc/include/glrc.h

        #multiprojector/frmprojector.h
        #multiprojector/frmprojector.cpp
        #multiprojector/objloader.h
        #multiprojector/objloader.cpp
        #multiprojector/channelhak.h
        #multiprojector/channelhak.cpp

        catalogpages/messierwindow.h
        catalogpages/messierwindow.cpp

        catalogpages/messierimage.h
        catalogpages/messierimage.cpp

        catalogpages/constellationwindow.h
        catalogpages/constellationwindow.cpp

        catalogpages/constsettingwindow.h
        catalogpages/constsettingwindow.cpp        

        )


IF(WIN32)
        SET(shiraplayer_lib_SRCS ${shiraplayer_lib_SRCS}
                external/kdewin32/basetyps.h
                external/kdewin32/byteswap.h
                external/kdewin32/comcat.h
                external/kdewin32/dirent.h
                external/kdewin32/docobj.h
                external/kdewin32/errno.h
                external/kdewin32/fcntl.h
                external/kdewin32/grp.h
                external/kdewin32/ifaddrs.h
                external/kdewin32/langinfo.h
                external/kdewin32/mshtml.h
                external/kdewin32/netdb.h
                external/kdewin32/nl_types.h
                external/kdewin32/oaidl.h
                external/kdewin32/objfwd.h
                external/kdewin32/objidl.h
                external/kdewin32/ocidl.h
                external/kdewin32/olectl.h
                external/kdewin32/oleidl.h
                external/kdewin32/pwd.h
                external/kdewin32/signal.h
                external/kdewin32/stdio.h
                external/kdewin32/stdlib.h
                external/kdewin32/string.h
                external/kdewin32/strings.h
                external/kdewin32/syslog.h
                external/kdewin32/time.h
                external/kdewin32/unistd.h
                external/kdewin32/unknwn.h
                external/kdewin32/wchar.h
                external/kdewin32/arpa/inet.h
                external/kdewin32/asm/byteorder.h
                external/kdewin32/net/if.h
                external/kdewin32/netinet/in.h
                external/kdewin32/netinet/tcp.h
                external/kdewin32/sys/ioctl.h
                external/kdewin32/sys/mman.h
                external/kdewin32/sys/resource.h
                external/kdewin32/sys/select.h
                external/kdewin32/sys/signal.h
                external/kdewin32/sys/socket.h
                external/kdewin32/sys/stat.h
                external/kdewin32/sys/time.h
                external/kdewin32/sys/times.h
                external/kdewin32/sys/types.h
                external/kdewin32/sys/uio.h
                external/kdewin32/sys/un.h
                external/kdewin32/sys/unistd.h
                external/kdewin32/sys/utsname.h
                external/kdewin32/sys/wait.h

                external/kdewin32/bind/inet_ntop.c
                external/kdewin32/bind/inet_pton.c
                external/kdewin32/dirent.c
                external/kdewin32/errno.c
                external/kdewin32/fsync.c
                external/kdewin32/getenv.c
                external/kdewin32/grp.c
                external/kdewin32/inet.c
                external/kdewin32/mmap.c
                external/kdewin32/nl_langinfo.c
                external/kdewin32/net.c
                external/kdewin32/pwd.c
                external/kdewin32/realpath.c
                external/kdewin32/resource.c
                external/kdewin32/signal.c
                external/kdewin32/stdlib.c
                external/kdewin32/string.c
                external/kdewin32/strptime.c
                external/kdewin32/syslog.c
                external/kdewin32/time.c
                external/kdewin32/uname.c
                external/kdewin32/unistd.c
        )
ENDIF(WIN32)

IF(APPLE)
  SET(shiraplayer_lib_SRCS
        ${shiraplayer_lib_SRCS})
ENDIF(APPLE)

SET(shiraplayer_exe_SRCS  main.cpp )

################# compiles resources files ############
SET(shiraplayer_RES ${CMAKE_SOURCE_DIR}/data/mainRes.qrc)
QT5_ADD_RESOURCES(shiraplayer_RES_CXX ${shiraplayer_RES})

################## compiles moc files ############
## Add here all the files which require moc compilation
## that is, all the headers with SIGNAL/SLOTS/PLUGIN_INTERFACE etc..
#SET(shiraplayer_MOC_HDRS
#        core/StelAudioMgr.hpp
#        core/StelModuleMgr.hpp
#        core/StelObserver.hpp
#        core/StelApp.hpp
#        core/StelNavigator.hpp
#        core/StelLocationMgr.hpp
#        core/StelCore.hpp
#        core/StelAppGraphicsWidget.hpp
#        core/StelTexture.hpp
#        core/StelMovementMgr.hpp
#        core/MultiLevelJsonBase.hpp
#        core/StelSkyImageTile.hpp
#        core/StelPresentImageTile.hpp
#        core/StelSkyPolygon.hpp
#        core/StelSkyDrawer.hpp
#        core/StelSkyLayerMgr.hpp
#        core/StelPresentMgr.hpp
#        core/StelSkyLayer.hpp
#        core/StelSkyCultureMgr.hpp
#        core/SimbadSearcher.hpp
#        modules/ConstellationMgr.hpp
#        modules/GridLinesMgr.hpp
#        modules/LabelMgr.hpp
#        modules/LandscapeMgr.hpp
#        modules/MeteorMgr.hpp
#        modules/MilkyWay.hpp
#        modules/NebulaMgr.hpp
#        modules/ScreenImageMgr.hpp
#        modules/SolarSystem.hpp
#        modules/StarMgr.hpp
#        modules/Telescope.hpp
#        modules/TelescopeMgr.hpp
#        scripting/StelScriptMgr.hpp
#        scripting/StelMainScriptAPI.hpp
#        scripting/StelMainScriptAPIProxy.hpp
#        socketutils/mysocket.h
#        socketutils/rsync.h
#        socketutils/sntpclient.h
#        StelMainGraphicsView.hpp
#        shiraplayerform.hpp
#        StelMainWindow.hpp
#        warping/channel.h
#        modules/networkcommands.h
#        videoutils/VideoFile.h
#        videoutils/AudioFile.h
#        recordutils/glthread.h
#        licenceutils/qlisansform.h
#        presenter/qplayimagewidget.h
#        presenter/qimagepropwidget.h
#        presenter/qmediabrowserwidget.h
#        presenter/VideoSource.h
#        previewutil/previewform.hpp
#        freehand/tupcolorpalette.h
#        freehand/tupslider.h
#        freehand/tupcolorwidget.h
#        freehand/tuppenpreviewcanvas.h

#)
# After this call, shiraplayer_MOC_SRCS = moc_shiraplayer.cxx
QT5_WRAP_CPP(shiraplayer_MOC_SRCS ${shiraplayer_MOC_HDRS})


#############################################################################################
############################# Standard GUI plugin compilation ###############################
#############################################################################################
IF (GUI_MODE STREQUAL "Standard")
        SET(shiraplayer_gui_SRCS
                gui/StelGui.hpp
                gui/StelGui.cpp
                gui/SkyGui.hpp
                gui/SkyGui.cpp
                gui/StelGuiItems.hpp
                gui/StelGuiItems.cpp
                gui/HelpDialog.hpp
                gui/HelpDialog.cpp
                gui/Dialog.hpp
                gui/Dialog.cpp
                gui/MapLabel.hpp
                gui/MapLabel.cpp
                gui/AngleSpinBox.hpp
                gui/AngleSpinBox.cpp
                gui/LocationDialog.hpp
                gui/LocationDialog.cpp
                gui/DateTimeDialog.hpp
                gui/DateTimeDialog.cpp
                gui/ViewDialog.hpp
                gui/ViewDialog.cpp
                gui/SearchDialog.hpp
                gui/SearchDialog.cpp
                gui/ConfigurationDialog.hpp
                gui/ConfigurationDialog.cpp
                gui/StelDialog.hpp
                gui/StelDialog.cpp
                gui/servermanager.h
                gui/servermanager.cpp
                gui/recordmanager.hpp
                gui/recordmanager.cpp

                gui/stellamanager.hpp
                gui/stellamanager.cpp

                gui/splashScreen.h
                gui/splashScreen.cpp

                licenceutils/qlisansform.h
                licenceutils/qlisansform.cpp

                presenter/qimagepropwidget.h
                presenter/qimagepropwidget.cpp

                previewutil/previewform.hpp
                previewutil/previewform.cpp

                #multiprojector/projsettingdialog.h
                #multiprojector/projsettingdialog.cpp

                catalogpages/messierwindow.h
                catalogpages/messierwindow.cpp

                catalogpages/constellationwindow.h
                catalogpages/constellationwindow.cpp

                catalogpages/constsettingwindow.h
                catalogpages/constsettingwindow.cpp

                gui/planetszoomdialog.h
                gui/planetszoomdialog.cpp

                gui/landscapeManager.h
                gui/landscapeManager.cpp

                gui/audiomanagerdialog.h
                gui/audiomanagerdialog.cpp

                gui/illimunationdialog.h
                gui/illimunationdialog.cpp

                videoutils/embedaudiowarning.h
                videoutils/embedaudiowarning.cpp

                )

        ################# compiles GUI moc files ############
        SET(shiraplayer_gui_MOC_HDRS
                gui/StelGui.hpp
                gui/SkyGui.hpp
                gui/StelGuiItems.hpp
                gui/HelpDialog.hpp
                gui/Dialog.hpp
                gui/MapLabel.hpp
                gui/AngleSpinBox.hpp
                gui/LocationDialog.hpp
                gui/DateTimeDialog.hpp
                gui/ViewDialog.hpp
                gui/SearchDialog.hpp
                gui/ConfigurationDialog.hpp
                gui/StelDialog.hpp
                gui/servermanager.h
                gui/stellamanager.hpp
                gui/recordmanager.hpp                
                gui/splashScreen.h
                licenceutils/qlisansform.h
                presenter/qimagepropwidget.h
                previewutil/previewform.hpp
                flybyutil/flybymanager.h
                #multiprojector/projsettingdialog.h
                catalogpages/messierwindow.h
                catalogpages/messierimage.h
                catalogpages/constellationwindow.h
                catalogpages/constsettingwindow.h
                gui/planetszoomdialog.h
                gui/landscapeManager.h
                gui/audiomanagerdialog.h
                gui/illimunationdialog.h
                videoutils/embedaudiowarning.h)

        ################# compiles .ui files ############
        SET(shiraplayer_UIS
                gui/locationDialogGui.ui
                gui/helpDialogGui.ui
                gui/dateTimeDialogGui.ui
                gui/viewDialog.ui
                gui/searchDialogGui.ui
                gui/configurationDialog.ui
                gui/servermanager.ui
                gui/stellamanager.ui
                gui/recordmanager.ui
                gui/splashScreen.ui
                licenceutils/qlisansform.ui
                presenter/qimagepropwidget.ui
                previewutil/previewform.ui
                flybyutil/flybymanager.ui
                #multiprojector/projsettingdialog.ui
                catalogpages/messierwindow.ui
                catalogpages/constellationwindow.ui
                catalogpages/constsettingwindow.ui
                gui/planetszoomdialog.ui
                gui/landscapeManager.ui
                gui/audiomanagerdialog.ui
                gui/illimunationdialog.ui
                videoutils/embedaudiowarning.ui)
        IF(ENABLE_SCRIPT_CONSOLE)
                SET(shiraplayer_gui_SRCS ${shiraplayer_gui_SRCS}
                        gui/ScriptConsole.hpp
                        gui/ScriptConsole.cpp
                        gui/StelScriptSyntaxHighlighter.hpp
                        gui/StelScriptSyntaxHighlighter.cpp)
                SET(shiraplayer_gui_MOC_HDRS ${shiraplayer_gui_MOC_HDRS}
                        gui/ScriptConsole.hpp
                        gui/StelScriptSyntaxHighlighter.hpp)
                SET(shiraplayer_UIS ${shiraplayer_UIS}
                        gui/scriptConsole.ui)
        ENDIF(ENABLE_SCRIPT_CONSOLE)

        QT5_WRAP_UI(shiraplayer_UIS_H ${shiraplayer_UIS})

        # compiles resources files
        SET(shiraplayer_gui_RES ${CMAKE_SOURCE_DIR}/data/gui/guiRes.qrc)
        QT5_ADD_RESOURCES(shiraplayer_gui_RES_CXX ${shiraplayer_gui_RES})

        # After this call, shiraplayer_gui_MOC_SRCS = moc_shiraplayer_gui.cxx
        QT5_WRAP_CPP(shiraplayer_gui_MOC_SRCS ${shiraplayer_gui_MOC_HDRS})

        SET(extLinkerOption ${QT_QTSCRIPT_LIBRARY} ${OPENGL_LIBRARIES})

        # Compile the static library
        ADD_LIBRARY(StelGuiLib STATIC ${shiraplayer_gui_SRCS} ${shiraplayer_gui_MOC_SRCS} ${shiraplayer_UIS_H})
        TARGET_LINK_LIBRARIES(StelGuiLib ${StelMain} ${extLinkerOption})
        QT5_USE_MODULES(StelGuiLib Core Gui Widgets Xml Concurrent Network OpenGL Script Declarative)
        SET_TARGET_PROPERTIES(StelGuiLib PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN")
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} StelGuiLib)
        ADD_DEPENDENCIES(AllStaticPlugins StelGuiLib)

#        # Compile the static library
#        ADD_LIBRARY(StelGuiLib STATIC ${shiraplayer_gui_SRCS} ${shiraplayer_gui_MOC_SRCS} ${shiraplayer_UIS_H})
#        TARGET_LINK_LIBRARIES(StelGuiLib ${StelMain} ${extLinkerOption})
#        QT5_USE_MODULES(StelGuiLib Core Gui Widgets Xml Concurrent Network OpenGL Script Declarative)
#        SET_TARGET_PROPERTIES(StelGuiLib PROPERTIES COMPILE_FLAGS "-fPIC -DQT_STATICPLUGIN")
#        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} ${CMAKE_BINARY_DIR}/src/${CMAKE_CFG_INTDIR}/libStelGuiLib.a)
#        ADD_DEPENDENCIES(AllStaticPlugins StelGuiLib)
ENDIF()

#############################################################################################
############################ Standard NO GUI plugin compilation #############################
#############################################################################################
IF (GUI_MODE STREQUAL "None")
        SET(shiraplayer_nogui_SRCS
                noGui/StelNoGui.hpp
                noGui/StelNoGui.cpp)
        SET(shiraplayer_nogui_MOC_HDRS
                noGui/StelNoGui.hpp)
        QT5_WRAP_CPP(shiraplayer_nogui_MOC_SRCS ${shiraplayer_nogui_MOC_HDRS})
        ADD_LIBRARY(StelNoGuiLib STATIC ${shiraplayer_nogui_SRCS} ${shiraplayer_nogui_MOC_SRCS})
        TARGET_LINK_LIBRARIES(StelNoGuiLib ${extLinkerOption})
        SET_TARGET_PROPERTIES(StelNoGuiLib PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN")
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} ${CMAKE_BINARY_DIR}/src/libStelNoGuiLib.a)
ENDIF()


#############################################################################################
############################# External GUI plugin compilation ###############################
#############################################################################################
IF (GUI_MODE STREQUAL "External")
        SET(EXTERNAL_GUI_SRC_PATH "../../MyGui/" CACHE PATH "The location of the plugin main directory i.e. the one containing the top level CMakeLists.txt")
        ADD_SUBDIRECTORY( ${EXTERNAL_GUI_SRC_PATH} externalGui )
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} ${CMAKE_BINARY_DIR}/src/externalGui/src/libExternalGui.a)
        SET(EXTERNAL_GUI_PLUGIN_EXTRALIBS CACHE STRING "Additionnal link options needed for the GUI plugin")
        SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${EXTERNAL_GUI_PLUGIN_EXTRALIBS})
ENDIF()


###############################################################################################
################## Static plugins need to be known by the file main.cpp at compilation time
################## so add the definitions to the compilation.
################## Also add the full path to the .a libraries at compilation time.
IF (BUILD_STATIC_PLUGINS)
  IF (USE_PLUGIN_HELLOSTELMODULE)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/HelloStelModule/src/${CMAKE_CFG_INTDIR}/libHelloStelModule.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_HELLOSTELMODULE)
  ENDIF()

  IF (USE_PLUGIN_ANGLEMEASURE)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/AngleMeasure/src/${CMAKE_CFG_INTDIR}/libAngleMeasure.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_ANGLEMEASURE)
  ENDIF()

  IF (USE_PLUGIN_COMPASSMARKS)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/CompassMarks/src/${CMAKE_CFG_INTDIR}/libCompassMarks.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_COMPASSMARKS)
  ENDIF()

  IF (USE_PLUGIN_SATELLITES)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/Satellites/src/${CMAKE_CFG_INTDIR}/libSatellites.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_SATELLITES)
  ENDIF()

  IF (USE_PLUGIN_TELESCOPECONTROL)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/TelescopeControl/src/${CMAKE_CFG_INTDIR}/libTelescopeControl.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_TELESCOPECONTROL)
  ENDIF()

  IF (USE_PLUGIN_LOGBOOK)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/LogBook/src/${CMAKE_CFG_INTDIR}/libLogBook.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_LOGBOOK)
        SET(QT_USE_QTSQL TRUE)
  ENDIF()

  IF (USE_PLUGIN_OCULARS)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/Oculars/src/${CMAKE_CFG_INTDIR}/libOculars.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_OCULARS)
        SET(QT_USE_QTSQL TRUE)
  ENDIF()

  IF (USE_PLUGIN_TEXTUSERINTERFACE)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/TextUserInterface/src/${CMAKE_CFG_INTDIR}/libTextUserInterface.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_TEXTUSERINTERFACE)
  ENDIF()

  IF (USE_PLUGIN_SVMT)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} ${CMAKE_BINARY_DIR}/plugins/svmt/src/${CMAKE_CFG_INTDIR}/libSVMT.a ${TOKYOC_LIBRARIES})
        IF(APPLE)
          SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "-framework QtDeclarative")
        ELSE()
          SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "/usr/local/Trolltech/Qt-4.7.0/lib/libQtDeclarative.so")
        ENDIF()
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_SVMT)
  ENDIF()

  IF (USE_PLUGIN_VIRGO)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} ${CMAKE_BINARY_DIR}/plugins/VirGO/src/${CMAKE_CFG_INTDIR}/libVirGO.a ${CFITSIO_LIBRARIES})
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_VIRGO)
  ENDIF()

  IF (USE_PLUGIN_SCENERY3D)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/Scenery3d/src/${CMAKE_CFG_INTDIR}/libScenery3d.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_SCENERY3D)
  ENDIF()

  IF (USE_PLUGIN_SHIRA3DMODEL)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/Shira3DModel/src/${CMAKE_CFG_INTDIR}/libShira3DModel.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_SHIRA3DMODEL)
  ENDIF()

  IF(USE_PLUGIN_SHIRAVIDEOMANAGER)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/ShiraVideoManager/src/${CMAKE_CFG_INTDIR}/libShiraVideoManager.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_SHIRAVIDEOMANAGER)
  ENDIF()

  IF (USE_PLUGIN_SOLARSYSTEMEDITOR)
        SET(shiraplayer_STATIC_PLUGINS_LIBRARIES ${shiraplayer_STATIC_PLUGINS_LIBRARIES} "${CMAKE_BINARY_DIR}/plugins/SolarSystemEditor/src/${CMAKE_CFG_INTDIR}/libSolarSystemEditor.a")
        ADD_DEFINITIONS(-DUSE_STATIC_PLUGIN_SOLARSYSTEMEDITOR)
  ENDIF()

ENDIF()


#############################################################################################
################################ Build main program #########################################
#############################################################################################

SET(extLinkerOption ${QT_LIBRARIES} ${QT_QTSCRIPT_LIBRARY} ${OPENGL_LIBRARIES} ${INTL_LIBRARIES} ${ZLIB_LIBRARIES}
                    ${ICONV_LIBRARIES} ${FREETYPE_LIBRARIES}
                    ${FFMPEG_AVCORE_LIBRARY} ${FFMPEG_AVCODEC_LIBRARY} ${FFMPEG_AVDEVICE_LIBRARY}
                    ${FFMPEG_AVFORMAT_LIBRARY} ${FFMPEG_AVUTIL_LIBRARY} ${FFMPEG_SWSCALE_LIBRARY} ${FFMPEG_SWRESAMPLE_LIBRARY}
                    ${SDL_LIBRARY} ${SDL_MAIN_LIBRARY} ${SHIRAPROJECTOR_LIBRARY})


IF(GENERATE_STELMAINLIB)
 ADD_LIBRARY(stelMain SHARED ${shiraplayer_lib_SRCS} ${shiraplayer_MOC_SRCS} ${shiraplayer_RES_CXX})
 TARGET_LINK_LIBRARIES(stelMain ${extLinkerOption} ${shiraplayer_STATIC_PLUGINS_LIBRARIES})
 INSTALL(TARGETS stelMain DESTINATION lib)
 ADD_EXECUTABLE(shiraplayer ${shiraplayer_exe_SRCS})
 #stelMain should be after the plug-ins, otherwise the build crashes
 TARGET_LINK_LIBRARIES(shiraplayer ${shiraplayer_STATIC_PLUGINS_LIBRARIES} stelMain ${extLinkerOption})
ELSE()
 ADD_EXECUTABLE(shiraplayer ${shiraplayer_lib_SRCS} ${shiraplayer_MOC_SRCS} ${shiraplayer_RES_CXX} ${shiraplayer_exe_SRCS})
 TARGET_LINK_LIBRARIES(shiraplayer ${extLinkerOption} ${shiraplayer_STATIC_PLUGINS_LIBRARIES})
 IF(APPLE)
# SET_TARGET_PROPERTIES(shiraplayer PROPERTIES LINK_FLAGS " -dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/works/OpenGL.framework/Versions/A/Libraries/libGL.dylib -framework Cocoa -framework QtDeclarative")
SET_TARGET_PROPERTIES(shiraplayer PROPERTIES LINK_FLAGS " -framework Cocoa -framework OpenGL")

 ENDIF()
ENDIF()

ADD_DEPENDENCIES(shiraplayer AllStaticPlugins)

INSTALL(TARGETS shiraplayer DESTINATION bin)



#############################################################################################
################################## Build unit tests #########################################
#############################################################################################

#SET(extLinkerOptionTest ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${ICONV_LIBRARIES} ${INTL_LIBRARIES} ${ZLIB_LIBRARIES} ${FFMPEG_LIBRARIES})

# Custom target used to build all tests at once
ADD_CUSTOM_TARGET(buildTests)

SET(tests_testDates_SRCS
        core/StelUtils.cpp
        core/StelUtils.hpp
        tests/testDates.cpp)
QT5_WRAP_CPP(testDates_MOC_SRCS tests/testDates.hpp)
ADD_EXECUTABLE(testDates EXCLUDE_FROM_ALL ${tests_testDates_SRCS} ${testDates_MOC_SRCS})
TARGET_LINK_LIBRARIES(testDates ${extLinkerOptionTest})
ADD_DEPENDENCIES(buildTests testDates)

SET(tests_testStelFileMgr_SRCS
        core/StelFileMgr.cpp
        core/StelFileMgr.hpp
        tests/testStelFileMgr.cpp)
QT5_WRAP_CPP(testStelFileMgr_MOC_SRCS tests/testStelFileMgr.hpp)
ADD_EXECUTABLE(testStelFileMgr EXCLUDE_FROM_ALL ${tests_testStelFileMgr_SRCS} ${testStelFileMgr_MOC_SRCS})
TARGET_LINK_LIBRARIES(testStelFileMgr ${extLinkerOptionTest})
ADD_DEPENDENCIES(buildTests testStelFileMgr)

SET(tests_testStelSphereGeometry_SRCS
        tests/testStelSphereGeometry.hpp
        tests/testStelSphereGeometry.cpp
        core/StelSphereGeometry.hpp
        core/StelSphereGeometry.cpp
        core/OctahedronPolygon.hpp
        core/OctahedronPolygon.cpp
        core/StelJsonParser.hpp
        core/StelJsonParser.cpp
        core/StelUtils.cpp
        core/StelUtils.hpp
        ${glues_lib_SRCS})
QT5_WRAP_CPP(testStelSphereGeometry_MOC_SRCS tests/testStelSphereGeometry.hpp)
ADD_EXECUTABLE(testStelSphereGeometry EXCLUDE_FROM_ALL ${tests_testStelSphereGeometry_SRCS} ${testStelSphereGeometry_MOC_SRCS})
TARGET_LINK_LIBRARIES(testStelSphereGeometry ${extLinkerOptionTest} ${QT_QTOPENGL_LIBRARY})
ADD_DEPENDENCIES(buildTests testStelSphereGeometry)

SET(tests_testStelSphericalIndex_SRCS
        tests/testStelSphericalIndex.hpp
        tests/testStelSphericalIndex.cpp
        core/StelSphericalIndex.hpp
        core/StelSphericalIndex.cpp
        core/StelSphereGeometry.hpp
        core/StelSphereGeometry.cpp
        core/OctahedronPolygon.hpp
        core/OctahedronPolygon.cpp
        core/StelJsonParser.hpp
        core/StelJsonParser.cpp
        core/StelUtils.cpp
        core/StelUtils.hpp
        ${glues_lib_SRCS})
QT5_WRAP_CPP(tests_testStelSphericalIndex_MOC_SRCS tests/testStelSphericalIndex.hpp)
ADD_EXECUTABLE(testStelSphericalIndex EXCLUDE_FROM_ALL ${tests_testStelSphericalIndex_SRCS} ${tests_testStelSphericalIndex_MOC_SRCS})
TARGET_LINK_LIBRARIES(testStelSphericalIndex ${extLinkerOptionTest} ${QT_QTOPENGL_LIBRARY})
ADD_DEPENDENCIES(buildTests testStelSphericalIndex)

SET(tests_testStelJsonParser_SRCS
        tests/testStelJsonParser.hpp
        tests/testStelJsonParser.cpp
        core/StelJsonParser.hpp
        core/StelJsonParser.cpp)
QT5_WRAP_CPP(tests_testStelJsonParser_MOC_SRCS tests/testStelJsonParser.hpp)
ADD_EXECUTABLE(testStelJsonParser EXCLUDE_FROM_ALL ${tests_testStelJsonParser_SRCS} ${tests_testStelJsonParser_MOC_SRCS})
TARGET_LINK_LIBRARIES(testStelJsonParser ${extLinkerOptionTest})
ADD_DEPENDENCIES(buildTests testStelJsonParser)

SET(tests_testStelVertexArray_SRCS
  tests/testStelVertexArray.hpp
  tests/testStelVertexArray.cpp
  core/StelVertexArray.hpp)
QT5_WRAP_CPP(tests_testStelVertexArray_MOC_SRCS tests/testStelVertexArray.hpp)
ADD_EXECUTABLE(testStelVertexArray EXCLUDE_FROM_ALL ${tests_testStelVertexArray_SRCS} ${tests_testStelVertexArray_MOC_SRCS})
TARGET_LINK_LIBRARIES(testStelVertexArray ${extLinkerOptionTest})
ADD_DEPENDENCIES(buildTests testStelVertexArray)

SET(tests_testRefractionExtinction_SRCS
  tests/testRefractionExtinction.hpp
  tests/testRefractionExtinction.cpp
  core/RefractionExtinction.hpp
  core/RefractionExtinction.cpp)
QT5_WRAP_CPP(tests_testRefractionExtinction_MOC_SRCS tests/testRefractionExtinction.hpp)
ADD_EXECUTABLE(testRefractionExtinction EXCLUDE_FROM_ALL ${tests_testRefractionExtinction_SRCS} ${tests_testRefractionExtinction_MOC_SRCS})
TARGET_LINK_LIBRARIES(testRefractionExtinction ${extLinkerOptionTest})
ADD_DEPENDENCIES(buildTests testRefractionExtinction)

ADD_CUSTOM_TARGET(tests COMMENT "Run the shiraplayer unit tests")
# ADD_CUSTOM_COMMAND(TARGET tests POST_BUILD COMMAND ./testDates WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/)
ADD_CUSTOM_COMMAND(TARGET tests POST_BUILD COMMAND ./testStelFileMgr WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/)
ADD_CUSTOM_COMMAND(TARGET tests POST_BUILD COMMAND ./testStelSphereGeometry WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/)
ADD_CUSTOM_COMMAND(TARGET tests POST_BUILD COMMAND ./testStelSphericalIndex WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/)
ADD_CUSTOM_COMMAND(TARGET tests POST_BUILD COMMAND ./testStelJsonParser WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/)
ADD_CUSTOM_COMMAND(TARGET tests POST_BUILD COMMAND ./testStelVertexArray WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/)
ADD_CUSTOM_COMMAND(TARGET tests POST_BUILD COMMAND ./testRefractionExtinction WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src/)
ADD_DEPENDENCIES(tests buildTests)

ADD_DEFINITIONS(-DSHIRAPLAYER_PRE) #DSHIRAPLAYER_PRE veya DSHIRAPLAYER_PRO
#set(CMAKE_CXX_FLAGS "-g -Wall") #debug icin
