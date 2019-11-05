#ifndef PROGRAM_H
#define PROGRAM_H

#include <QObject>

class Program
{


public:
    explicit Program();
    ~Program();


public slots:
    void setFullUrl(QString);
    QString fullUrl();
    QString name();
    QString url();
    void setArguments(QStringList);
    QStringList arguments();
    void setScriptUrl(QString);
    QString scriptUrl();
    bool hasArguments();
    bool hasScript();

signals:


private slots:
    void setName();
    void setUrl();

private:
    QString m_fullUrl;
    QString m_name;
    QString m_url;
    QStringList m_arguments;
    QString m_scriptUrl;
    bool m_hasArguments;
    bool m_hatScript;
};

#endif // PROGRAM_H
