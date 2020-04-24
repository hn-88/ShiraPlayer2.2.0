#ifndef SHIRAPROJECTOR_H
#define SHIRAPROJECTOR_H

#include "shiraprojector_global.h"
#include "prjchannel.h"

#include <QGLWidget>

class SHIRAPROJECTORSHARED_EXPORT Shiraprojector
{

public:
    Shiraprojector();
    void setprjTex(GLuint tex);
    void LoadProjectors(QGLWidget *sharedGL, bool show);
    void CloseProjectors();
    double getViewportRes();
    bool getUseBuffer();
    bool getIsFramelessWindow();

    static Shiraprojector& getInstance() {Q_ASSERT(singleton); return *singleton;}

    QList<QGLWidget*> getProjectors(){ return m_projectors;}
    QGLWidget * getSharedGL() { return prjsharedGL;}
    void AddProjector(PrjChannel* ch);
    void RemoveChannel(int index);
    void ClearProjectors();
    void ShowProjSettingsDialog();
    QString getPluginName();
    bool isRegistered();
    bool isEnabled();

    QList<PrjChannel*> m_pChannels;    //!< PrjChannel objects.
    PrjChannel* m_pSelectedChannel;    //!< Selected channel object.
private:

    static Shiraprojector* singleton;
    QList<QGLWidget*> m_projectors;
    double viewportRes;
    bool framelessProjWindow;
    void loadProjSettings(char *filename);
    QGLWidget *prjsharedGL;
    bool registered;
    bool enabled;
};

#endif // SHIRAPROJECTOR_H
