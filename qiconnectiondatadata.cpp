#include "qiconnectiondata.h"
#include <QDebug>
#include <QStringList>

QiConnectionData::QiConnectionData(int socketDescriptor):socketId(socketDescriptor){
    qDebug()<<"PipeData contructed:"<<socketId;
    returnCode = -1;
}


void QiConnectionData::setRequestHeader(QByteArray header){
    header.replace("\r\n","\n");
    int i=0,l=header.length();
    //firstline
    i=header.indexOf('\n');
    if(i==-1){
        qDebug()<<"-------- invalid response header  ------"<<header;
        return;
    }
    QByteArray firstLine = header.left(i).simplified();
    QList<QByteArray> sigs = firstLine.split(' ');
    if(sigs.length() != 3){
        qDebug()<<"-------- invalid response header  ------"<<header;
        return;
    }
    requestMethod = sigs.at(0);
    fullUrl = sigs.at(1);
    protocol = sigs.at(2);

    // change http://aaa.com/a/b/c?d to /a/b/c?d
    path = "/";
    int n;
    //qDebug()<<"fullUrl="<<fullUrl;
    if(fullUrl.indexOf("://")!=-1){
        n = fullUrl.split("://")[1].indexOf("/");
        if(n!=-1 && n<fullUrl.length()-1){
            path = fullUrl.split("://")[1].mid(n);
        }
    }else{
        n = fullUrl.indexOf("/");
        if(n!=-1 && n<fullUrl.length()-1){
            path = fullUrl.mid(n);
        }
    }

    //TODO..

    requestRawDataToSend = QByteArray().append(requestMethod)
            .append(' ')
            .append(path)
            .append(' ')
            .append(protocol);
    requestRawDataToSend.append(header.mid(i));

    requestRawDataToSend.replace(QString("Proxy-Connection: keep-alive\n"),QByteArray(""));
    requestRawDataToSend.replace(QString("\n"),QByteArray("\r\n"));
    requestRawDataToSend.append(QByteArray("\r\n\r\n"));
    /*
    QByteArray test = requestRawDataToSend;
    test.replace("\r","\\r\r");
    test.replace("\n","\\n\n");
    qDebug()<<"test=\n"<<test;
    */
    //the rest..
    while(i<l){
        int j=header.indexOf('\n',i);
        if(j==-1){// last line
            j=l;
        }
        QByteArray line = header.mid(i,j-i);
        //qDebug()<<line;

        int splitIndex = line.indexOf(':');
        QByteArray name = QByteArray(line.left(splitIndex));
        QByteArray value = QByteArray(line.mid(splitIndex+1).trimmed());
        //setRequestHeader(name,value);
        if(name == QString("Host")){
            int d = value.indexOf(":");
            if(d!=-1){
                allRequestHeaders["Host"] = value.left(d);
                allRequestHeaders["Port"] = value.mid(d+1);
            }else{
                allRequestHeaders["Host"] = value;
                allRequestHeaders["Port"] = "80";
            }
        }else{
            allRequestHeaders[name] = value;
        }
        i=j+1;
    }
}
void QiConnectionData::setResponseHeader(QByteArray header){

    //TODO.. Ctrl+c & Ctrl+v from setRequestHeader
    header.replace("\r\n","\n");
    int i=0,l=header.length();

    //firstline
    //HTTP/1.1 302 Found
    i=header.indexOf('\n');
    Q_ASSERT(i!=-1);
    QByteArray firstLine = header.left(i).simplified();
    QList<QByteArray> sigs = firstLine.split(' ');
    Q_ASSERT(sigs.length() == 3);
    returnCode = sigs.at(1).simplified().toInt();

    while(i<l){
        int j=header.indexOf('\n',i);
        if(j==-1){// last line
            j=l;
        }
        QByteArray line = header.mid(i,j-i);
        //qDebug()<<line;

        int splitIndex = line.indexOf(':');
        QByteArray name = QByteArray(line.left(splitIndex));
        QByteArray value = QByteArray(line.mid(splitIndex+1).trimmed());
        allResponseHeaders[name]=value;
        //setRequestHeader(name,value);
        i=j+1;
    }
}


QByteArray QiConnectionData::getResponseHeader(QByteArray name)const{
    return allResponseHeaders[name];
}
QByteArray QiConnectionData::getResponseHeader()const{

}
QByteArray QiConnectionData::getResponseBody()const{

}

QByteArray QiConnectionData::getRequestHeader() const{
    if(requestRawData.isEmpty()){
        return requestRawData;
    }
    int i = requestRawData.indexOf("\r\n\r\n");
    if(i==-1){
        i = requestRawData.indexOf("\r\n");
    }
    if(i==-1){
        return requestRawData;
    }else{
        return requestRawData.left(i);
    }
}
QByteArray QiConnectionData::getRequestHeader(QByteArray name) const{
    return allRequestHeaders.value(name,QByteArray());
}
QByteArray QiConnectionData::getRequestBody()const{
    if(requestRawData.isEmpty()){
        return QByteArray();
    }
    int i = requestRawData.indexOf("\r\n\r\n");
    if(i!=-1){
        return requestRawData.mid(i+4);
    }else{
        i = requestRawData.indexOf("\r\n");
        if(i!=-1){
            return requestRawData.mid(i+2);
        }
    }
    return QByteArray();

}



void QiConnectionData::setRequestRawData(QByteArray request){//only for copy Ctor

}
void QiConnectionData::setResponseRawData(QByteArray response){//only for copy Ctor

}
