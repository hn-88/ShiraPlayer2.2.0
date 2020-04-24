#ifndef PRJCHANNEL_H
#define PRJCHANNEL_H

#include <QObject>
#include <QtXml/QtXml>

class PrjChannel : public QObject
{
    Q_OBJECT
public:
    PrjChannel();
    void setName(const QString& name){ m_name = name;}
    QString getName(){ return m_name;}
    void setWidth(int val) { width = val;}
    void setHeight(int val) {height = val; }
    void setTop(int val) { top = val;}
    void setLeft(int val) { left = val;}
    int getWidth(){ return width;}
    int getHeight(){return height;}
    int getLeft(){ return left;}
    int getTop() { return top;}
    void setObjFileName(const QString &file){ objFileName = file;}
    QString getObjFileName(){return objFileName;}
    void setBlendFileName(const QString &file){ blendFileName = file;}
    QString getBlendFileName(){return blendFileName;}

    QDomElement domElement(const QString& name, QDomDocument& doc) const;
    bool initFromDOMElement(const QDomElement& element);

private:
    QString m_name;     //!< PrjChannel name.

    int width;
    int height;
    int top;
    int left;
    QString objFileName;
    QString blendFileName;

signals:

public slots:

};

#endif // PRJCHANNEL_H
