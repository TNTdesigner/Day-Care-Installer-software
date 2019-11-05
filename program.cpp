#include "program.h"

Program::Program()
{
    m_hatScript = false;
    m_hasArguments = false;
}

Program::~Program()
{

}

void Program::setFullUrl(QString fullUrl)
{
    m_fullUrl = fullUrl.replace('/',"\\");
    setUrl();
    setName();
}

QString Program::fullUrl()
{
    return m_fullUrl;
}

QString Program::name()
{
    return m_name;
}

QString Program::url()
{
    return m_url;
}

void Program::setArguments(QStringList arg)
{
    m_arguments = arg;
    if(m_arguments.count() != 0){
        m_hasArguments = true;
    }else{
        m_hasArguments = false;
    }
}

QStringList Program::arguments()
{
    return m_arguments;
}

void Program::setScriptUrl(QString url)
{
    m_scriptUrl = url;
    if(m_scriptUrl != ""){
        m_hatScript = true;
    }else{
        m_hatScript = false;
    }
}

QString Program::scriptUrl()
{
    return m_scriptUrl;
}

bool Program::hasArguments()
{
    return m_hasArguments;
}

bool Program::hasScript()
{
    return m_hatScript;
}

void Program::setName()
{
    QStringList splitted = m_fullUrl.split("\\");
    m_name = splitted[splitted.length()-1];
}

void Program::setUrl()
{
    m_url = m_fullUrl;
    int index = m_url.lastIndexOf("\\");
    m_url.chop(m_url.count() - index);
}
