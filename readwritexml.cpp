#include "readwritexml.h"

ReadWriteXml::ReadWriteXml(QObject *parent): QObject(parent)
{
}

ReadWriteXml::~ReadWriteXml() {

}

void ReadWriteXml::writeXml(QString filepath,TestResultEntry entry) {
    //打开或创建文件
    QFile file(filepath); //相对路径、绝对路径、资源路径都可以
    QDomDocument doc;
    QDomElement root;
    QDomText domtext;
    if(file.size() > 0) {
        if(!doc.setContent(&file)) {
            qDebug() << "!doc.setContent";
            file.close();
            return;
        }
        file.close();
    }

    if(doc.isNull()) {
        qDebug() << "doc.isNull()";
        //写入xml头部
        QDomProcessingInstruction instruction; //添加处理命令
        instruction=doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(instruction);
        //添加根节点
        root=doc.createElement("test_result");
        doc.appendChild(root);
        QDomElement total_num=doc.createElement("total_test_num"); //创建子元素
        domtext=doc.createTextNode(QString::number(entry.totalTestItems));
        total_num.appendChild(domtext);
        root.appendChild(total_num);
        QDomElement pass_num=doc.createElement("total_test_pass_num"); //创建子元素
        domtext=doc.createTextNode(QString::number(entry.totalPassItems));
        pass_num.appendChild(domtext);
        root.appendChild(pass_num);
    }else {
       // qDebug() << "!doc.isNull()";
        root= doc.firstChildElement();
        QDomNode domNode = root.firstChild(); //获得第一个子节点
        qDebug() << domNode.nodeName();
        while (!domNode.isNull())  //如果节点不空
        {
            if (domNode.isElement()) //如果节点是元素
            {
                QDomElement domElement = domNode.toElement(); //转换为元素，注意元素和节点是两个数据结构，其实差不多
                if(domElement.nodeName() == "total_test_num") {
                    qDebug() << "total_test_num" << entry.totalTestItems;
                    QDomNode oldnode = domElement.firstChild();     //标签之间的内容作为节点的子节点出现，得到原来的子节点
                    domElement.firstChild().setNodeValue(QString::number(entry.totalTestItems));   //用提供的value值来设置子节点的内容
                    QDomNode newnode = domElement.firstChild();     //值修改过后
                    domElement.replaceChild(newnode,oldnode);
                }
                if(domElement.nodeName() == "total_test_pass_num") {
                    QDomNode oldnode = domElement.firstChild();     //标签之间的内容作为节点的子节点出现，得到原来的子节点
                    domElement.firstChild().setNodeValue(QString::number(entry.totalPassItems));   //用提供的value值来设置子节点的内容
                    QDomNode newnode = domElement.firstChild();     //值修改过后
                    domElement.replaceChild(newnode,oldnode);
                }
             }
             domNode = domNode.nextSibling();
        }
    }
    QDomElement item=doc.createElement("item");
    item.setAttribute("id",entry.id); //方式一：创建属性  其中键值对的值可以是各种类型
    QDomAttr time=doc.createAttribute("time"); //方式二：创建属性 值必须是字符串
    time.setValue(entry.dateTime);
    item.setAttributeNode(time);

    QDomElement sn=doc.createElement("产品序列号"); //创建子元素
    QDomText text;
    text=doc.createTextNode(entry.SN);
    item.appendChild(sn);
    sn.appendChild(text);
    QDomElement result=doc.createElement("结果"); //创建子元素
    text=doc.createTextNode(entry.result);
    result.appendChild(text);
    item.appendChild(result);
    QDomElement hw=doc.createElement("硬件版本号"); //创建子元素
    text=doc.createTextNode(entry.hw);
    hw.appendChild(text);
    item.appendChild(hw);
    QDomElement sw=doc.createElement("软件版本号"); //创建子元素
    text=doc.createTextNode(entry.sw);
    sw.appendChild(text);
    item.appendChild(sw);
   /* QDomElement pn=doc.createElement("零件号"); //创建子元素
    text=doc.createTextNode(entry.produce_num);
    pn.appendChild(text);
    item.appendChild(pn);*/
    QDomElement nottofdistance=doc.createElement("未踩踏距离"); //创建子元素
    text=doc.createTextNode(QString::number(entry.nottofdistance));
    nottofdistance.appendChild(text);
    item.appendChild(nottofdistance);
    QDomElement tofdistance=doc.createElement("踩踏距离"); //创建子元素
    text=doc.createTextNode(QString::number(entry.tofdistance));
    tofdistance.appendChild(text);
    item.appendChild(tofdistance);
    QDomElement normal_9v=doc.createElement("工作9V电流"); //创建子元素
    text=doc.createTextNode(QString::number(entry.normal_current_value_9V));
    normal_9v.appendChild(text);
    item.appendChild(normal_9v);
    QDomElement normal_13v=doc.createElement("工作13V电流"); //创建子元素
    text=doc.createTextNode(QString::number(entry.normal_current_value_13V));
    normal_13v.appendChild(text);
    item.appendChild(normal_13v);
    QDomElement normal_15v=doc.createElement("工作16V电流"); //创建子元素
    text=doc.createTextNode(QString::number(entry.normal_current_value_15V));
    normal_15v.appendChild(text);
    item.appendChild(normal_15v);
    QDomElement idle_9v=doc.createElement("待机9V电流"); //创建子元素
    text=doc.createTextNode(QString::number(entry.idle_current_value_9V));
    idle_9v.appendChild(text);
    item.appendChild(idle_9v);
    QDomElement idle_13v=doc.createElement("待机13V电流"); //创建子元素
    text=doc.createTextNode(QString::number(entry.idle_current_value_13V));
    idle_13v.appendChild(text);
    item.appendChild(idle_13v);
    QDomElement idle_15v=doc.createElement("待机16V电流"); //创建子元素
    text=doc.createTextNode(QString::number(entry.idle_current_value_15V));
    idle_15v.appendChild(text);
    item.appendChild(idle_15v);
    QDomElement sleep_9v=doc.createElement("静态9V电流"); //创建子元素
    text=doc.createTextNode(QString::number(entry.sleep_current_value_9V));
    sleep_9v.appendChild(text);
    item.appendChild(sleep_9v);
    QDomElement sleep_13v=doc.createElement("静态13V电流"); //创建子元素
    text=doc.createTextNode(QString::number(entry.sleep_current_value_13V));
    sleep_13v.appendChild(text);
    item.appendChild(sleep_13v);
    QDomElement sleep_15v=doc.createElement("静态16V电流"); //创建子元素
    text=doc.createTextNode(QString::number(entry.sleep_current_value_15V));
    sleep_15v.appendChild(text);
    item.appendChild(sleep_15v);
    QDomElement diameter=doc.createElement("投影直径"); //创建子元素
    text=doc.createTextNode(entry.diamater);
    diameter.appendChild(text);
    item.appendChild(diameter);
    QDomElement brightness=doc.createElement("投影照度"); //创建子元素
    text=doc.createTextNode(entry.brightness);
    brightness.appendChild(text);
    item.appendChild(brightness);
    QDomElement distortion=doc.createElement("投影畸变"); //创建子元素
    text=doc.createTextNode(entry.distortion);
    distortion.appendChild(text);
    item.appendChild(distortion);
    root.appendChild(item);
    if(!file.open(QFile::WriteOnly|QFile::Truncate)) //可以用QIODevice，Truncate表示清空原来的内容
        return;
    //输出到文件
    QTextStream out_stream(&file);
    doc.save(out_stream,4); //缩进4格
    file.close();
}

void ReadWriteXml::writeXml(QString filepath,int totalTestNum, int totalPassNum,QList<TestResultEntry> list)
{
        //打开或创建文件
        QFile file(filepath); //相对路径、绝对路径、资源路径都可以
        QDomDocument doc;
        QDomElement root;
        QDomText domtext;
        if(file.size() > 0) {
            if(!doc.setContent(&file)) {
                qDebug() << "!doc.setContent";
                file.close();
                return;
            }
            file.close();
        }

        if(doc.isNull()) {
            qDebug() << "doc.isNull()";
            //写入xml头部
            QDomProcessingInstruction instruction; //添加处理命令
            instruction=doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
            doc.appendChild(instruction);
            //添加根节点
            root=doc.createElement("test_result");
            doc.appendChild(root);
            QDomElement total_num=doc.createElement("total_test_num"); //创建子元素
            domtext=doc.createTextNode(QString::number(totalTestNum));
            total_num.appendChild(domtext);
            root.appendChild(total_num);
            QDomElement pass_num=doc.createElement("total_test_pass_num"); //创建子元素
            domtext=doc.createTextNode(QString::number(totalPassNum));
            pass_num.appendChild(domtext);
            root.appendChild(pass_num);
        }else {
           // qDebug() << "!doc.isNull()";
            root= doc.firstChildElement();
            QDomNode domNode = root.firstChild(); //获得第一个子节点
            qDebug() << domNode.nodeName();
            while (!domNode.isNull())  //如果节点不空
            {
                if (domNode.isElement()) //如果节点是元素
                {
                    QDomElement domElement = domNode.toElement(); //转换为元素，注意元素和节点是两个数据结构，其实差不多
                    if(domElement.nodeName() == "total_test_num") {
                        qDebug() << "total_test_num" << totalTestNum;
                        QDomNode oldnode = domElement.firstChild();     //标签之间的内容作为节点的子节点出现，得到原来的子节点
                        domElement.firstChild().setNodeValue(QString::number(totalTestNum));   //用提供的value值来设置子节点的内容
                        QDomNode newnode = domElement.firstChild();     //值修改过后
                        domElement.replaceChild(newnode,oldnode);
                    }
                    if(domElement.nodeName() == "total_test_pass_num") {
                        QDomNode oldnode = domElement.firstChild();     //标签之间的内容作为节点的子节点出现，得到原来的子节点
                        domElement.firstChild().setNodeValue(QString::number(totalPassNum));   //用提供的value值来设置子节点的内容
                        QDomNode newnode = domElement.firstChild();     //值修改过后
                        domElement.replaceChild(newnode,oldnode);
                    }
                 }
                 domNode = domNode.nextSibling();
            }
        }

        //添加第一个子节点及其子元素
        for(TestResultEntry entry:list) {
            QDomElement item=doc.createElement("item");
            item.setAttribute("id",entry.id); //方式一：创建属性  其中键值对的值可以是各种类型
            QDomAttr time=doc.createAttribute("time"); //方式二：创建属性 值必须是字符串
            time.setValue(entry.dateTime);
            item.setAttributeNode(time);

            QDomElement sn=doc.createElement("产品序列号"); //创建子元素
            QDomText text;
            text=doc.createTextNode(entry.SN);
            item.appendChild(sn);
            sn.appendChild(text);
            QDomElement result=doc.createElement("结果"); //创建子元素
            text=doc.createTextNode(entry.result);
            result.appendChild(text);
            item.appendChild(result);
            QDomElement hw=doc.createElement("硬件版本号"); //创建子元素
            text=doc.createTextNode(entry.hw);
            hw.appendChild(text);
            item.appendChild(hw);
            QDomElement sw=doc.createElement("软件版本号"); //创建子元素
            text=doc.createTextNode(entry.sw);
            sw.appendChild(text);
            item.appendChild(sw);
            /*QDomElement pn=doc.createElement("零件号"); //创建子元素
            text=doc.createTextNode(entry.produce_num);
            pn.appendChild(text);
            item.appendChild(pn);*/
            QDomElement nottofdistance=doc.createElement("未踩踏距离"); //创建子元素
            text=doc.createTextNode(QString::number(entry.nottofdistance));
            nottofdistance.appendChild(text);
            item.appendChild(nottofdistance);
            QDomElement tofdistance=doc.createElement("踩踏距离"); //创建子元素
            text=doc.createTextNode(QString::number(entry.tofdistance));
            tofdistance.appendChild(text);
            item.appendChild(tofdistance);
            QDomElement normal_9v=doc.createElement("工作9V电流"); //创建子元素
            text=doc.createTextNode(QString::number(entry.normal_current_value_9V));
            normal_9v.appendChild(text);
            item.appendChild(normal_9v);
            QDomElement normal_13v=doc.createElement("工作13V电流"); //创建子元素
            text=doc.createTextNode(QString::number(entry.normal_current_value_13V));
            normal_13v.appendChild(text);
            item.appendChild(normal_13v);
            QDomElement normal_15v=doc.createElement("工作15V电流"); //创建子元素
            text=doc.createTextNode(QString::number(entry.normal_current_value_15V));
            normal_15v.appendChild(text);
            item.appendChild(normal_15v);
            QDomElement idle_9v=doc.createElement("待机9V电流"); //创建子元素
            text=doc.createTextNode(QString::number(entry.idle_current_value_9V));
            idle_9v.appendChild(text);
            item.appendChild(idle_9v);
            QDomElement idle_13v=doc.createElement("待机13V电流"); //创建子元素
            text=doc.createTextNode(QString::number(entry.idle_current_value_13V));
            idle_13v.appendChild(text);
            item.appendChild(idle_13v);
            QDomElement idle_15v=doc.createElement("待机15V电流"); //创建子元素
            text=doc.createTextNode(QString::number(entry.idle_current_value_15V));
            idle_15v.appendChild(text);
            item.appendChild(idle_15v);
            QDomElement sleep_9v=doc.createElement("静态9V电流"); //创建子元素
            text=doc.createTextNode(QString::number(entry.sleep_current_value_9V));
            sleep_9v.appendChild(text);
            item.appendChild(sleep_9v);
            QDomElement sleep_13v=doc.createElement("静态13V电流"); //创建子元素
            text=doc.createTextNode(QString::number(entry.sleep_current_value_13V));
            sleep_13v.appendChild(text);
            item.appendChild(sleep_13v);
            QDomElement sleep_15v=doc.createElement("静态15V电流"); //创建子元素
            text=doc.createTextNode(QString::number(entry.sleep_current_value_15V));
            sleep_15v.appendChild(text);
            item.appendChild(sleep_15v);
            root.appendChild(item);
        }
        if(!file.open(QFile::WriteOnly|QFile::Truncate)) //可以用QIODevice，Truncate表示清空原来的内容
            return;
        //输出到文件
        QTextStream out_stream(&file);
        doc.save(out_stream,4); //缩进4格
        file.close();

}
//读取基站配置文件数据
void ReadWriteXml::readXml(QString fileNamePath)
{
    //打开或创建文件
    //qDebug() <<"fileNamePath:" <<fileNamePath;
    QFile xmlFile(fileNamePath);
    if (!xmlFile.open(QFile::ReadOnly))
    {
        qDebug() << "open failed";
        return;
    }

    QDomDocument docXML;
    if (!docXML.setContent(&xmlFile))
    {
        xmlFile.close();
        return;
    }
    xmlFile.close();

    QDomElement xmlRoot = docXML.documentElement(); //返回根节点
   // qDebug() << xmlRoot.tagName();
    QDomNode domNode = xmlRoot.firstChild(); //获得第一个子节点
   // qDebug() << domNode.nodeName();
    while (!domNode.isNull())  //如果节点不空
    {
        if (domNode.isElement()) //如果节点是元素
        {
            QDomElement domElement = domNode.toElement(); //转换为元素，注意元素和节点是两个数据结构，其实差不多
            if(domElement.nodeName() == "total_test_num") {
                mTotalTestNum = domElement.toElement().text();
                //qDebug() <<"mTotalTestNum:" <<mTotalTestNum;
            }
            if(domElement.nodeName() == "total_test_pass_num") {
                mTotalPassNum = domElement.toElement().text();
                //qDebug() <<"mTotalPassNum:" <<mTotalPassNum;
            }
            QDomNodeList list = domElement.childNodes();
            for (int i = 0; i<list.count(); i++) //遍历子元素，count和size都可以用,可用于标签数计数
            {
                QDomNode nodeIndex = list.at(i);
                if (domNode.isElement())
                {
                    if(nodeIndex.nodeName() == "project_name") {
                        mProjectName = nodeIndex.toElement().text();
                    }
                   // qDebug() << nodeIndex.nodeName();
                    if(nodeIndex.nodeName() == "hw_version") {
                      //  qDebug() << nodeIndex.toElement().text();
                        mHwVersion = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "sw_version") {
                        //qDebug() << nodeIndex.toElement().text();
                        mSwVersion = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "tof_not_distance") {
                        //qDebug() << nodeIndex.toElement().text();
                        mNotTofDistance = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v9_normal_current_min") {
                        m9VNormalMinCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v13_normal_current_min") {
                        m13VNormalMinCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v15_normal_current_min") {
                        m15VNormalMinCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v9_sleep_current_min") {
                        m9VSleepMinCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v13_sleep_current_min") {
                        m13VSleepMinCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v15_sleep_current_min") {
                        m15VSleepMinCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v9_idle_current_min") {
                        m9VIdleMinCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v13_idle_current_min") {
                        m13VIdleMinCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v15_idle_current_min") {
                        m15VIdleMinCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v9_normal_current_max") {
                        m9VNormalMaxCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v13_normal_current_max") {
                        m13VNormalMaxCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v15_normal_current_max") {
                        m15VNormalMaxCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v9_sleep_current_max") {
                        m9VSleepMaxCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v13_sleep_current_max") {
                        m13VSleepMaxCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v15_sleep_current_max") {
                        m15VSleepMaxCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v9_idle_current_max") {
                        m9VIdleMaxCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v13_idle_current_max") {
                        m13VIdleMaxCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "v15_idle_current_max") {
                        m15VIdleMaxCur = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "float_vol") {
                        mFloatVol = nodeIndex.toElement().text();
                    }
                    if(nodeIndex.nodeName() == "tof_distance") {
                      //  qDebug() << nodeIndex.toElement().text();
                        //bool ok;
                       // mTofDistance = nodeIndex.toElement().text().toInt(&ok, 16);
                        mTofDistance = nodeIndex.toElement().text();
                    }
                    /*if(nodeIndex.nodeName() == "product_num") {
                       // qDebug() << nodeIndex.toElement().text();
                        mProductNum = nodeIndex.toElement().text();
                    }*/
                    QDomElement indexElement = nodeIndex.toElement();
                    if (indexElement.hasAttribute("name"))
                    {
                        //qDebug() << nodeIndex.nodeName() << ":" << nodeIndex.toElement().attribute("id") << "  " << nodeIndex.toElement().attribute("name");
                        if(!mProjectList.contains(nodeIndex.toElement().attribute("name"))) {
                            mProjectList << nodeIndex.toElement().attribute("name");
                        }
                    }
                }
            }
        }
        //下一个兄弟节点,nextSiblingElement()是下一个兄弟元素
        domNode = domNode.nextSibling();
        //qDebug() << domNode.nodeName();
    }

    for(int i = 0; i< mProjectList.size();++i)
    {
        QString tmp = mProjectList.at(i);
       // qDebug() << tmp;
    }
}

void ReadWriteXml::updateSettingXml(QString fileNamePath, ParamSettingEntry entry) {
    //打开或创建文件
   // qDebug() <<"fileNamePath:" <<fileNamePath;
    QFile xmlFile(fileNamePath);
    if (!xmlFile.open(QFile::ReadOnly))
    {
        qDebug() << "open failed";
        return;
    }

    QDomDocument docXML;
    if (!docXML.setContent(&xmlFile))
    {
        xmlFile.close();
        return;
    }
    xmlFile.close();

    QDomElement xmlRoot = docXML.documentElement(); //返回根节点
   // qDebug() << xmlRoot.tagName();
    QDomNode domNode = xmlRoot.firstChild(); //获得第一个子节点
   // qDebug() << domNode.nodeName();
    while (!domNode.isNull()){
        if (domNode.isElement()) //如果节点是元素
        {
            QDomElement domElement = domNode.toElement(); //转换为元素，注意元素和节点是两个数据结构，其实差不多
          //  qDebug()<< "domElement.nodeName():" << domElement.nodeName();
            if(domElement.nodeName() == "Project_list") {
               // QDomNode oldnode = domElement.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
               // domElement.firstChild().setNodeValue(entry.project_name);//用提供的value值来设置子节点的内容
               // QDomNode newnode = domElement.firstChild();//值修改过后
               // domElement.replaceChild(newnode,oldnode);
            }
            QDomNodeList list = domElement.childNodes();
            for (int i = 0; i<list.count(); i++) //遍历子元素，count和size都可以用,可用于标签数计数
            {
                QDomNode nodeIndex = list.at(i);
                if (domNode.isElement())
                {
                   // qDebug() << nodeIndex.nodeName();
                    if(nodeIndex.nodeName()== "hw_version") {
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.hw);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);//调用节点的replaceChild方法实现修改功能
                    }
                    if(nodeIndex.nodeName()== "sw_version") {
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.sw);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);

                    }
                    /*if(nodeIndex.nodeName()== "product_num"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.produce_num);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }*/
                    if(nodeIndex.nodeName()== "float_vol") {
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.vol_float);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v9_normal_current_min") {
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.normal_value_9V_min);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v13_normal_current_min") {
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.normal_value_13V_min);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v15_normal_current_min"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.normal_value_15V_min);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v9_sleep_current_min"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.sleep_value_9V_min);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v15_sleep_current_min"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.sleep_value_15V_min);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v13_sleep_current_min"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.sleep_value_13V_min);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v9_idle_current_min"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.idle_value_9V_min);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v15_idle_current_min"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.idle_value_15V_min);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v13_idle_current_min"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.idle_value_13V_min);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v9_normal_current_max") {
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.normal_value_9V_max);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v13_normal_current_max") {
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.normal_value_13V_max);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v15_normal_current_max"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.normal_value_15V_max);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v9_sleep_current_max"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.sleep_value_9V_max);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v15_sleep_current_max"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.sleep_value_15V_max);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v13_sleep_current_max"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.sleep_value_13V_max);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v9_idle_current_max"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.idle_value_9V_max);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v15_idle_current_max"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.idle_value_15V_max);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "v13_idle_current_max"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.idle_value_13V_max);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "tof_not_distance"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.nottofdistance);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                    if(nodeIndex.nodeName()== "tof_distance"){
                        QDomNode oldnode = nodeIndex.firstChild();//标签之间的内容作为节点的子节点出现，得到原来的子节点
                        nodeIndex.firstChild().setNodeValue(entry.tofdistance);//用提供的value值来设置子节点的内容
                        QDomNode newnode = nodeIndex.firstChild();//值修改过后
                        nodeIndex.replaceChild(newnode,oldnode);
                    }
                }
            }
         }
         domNode = domNode.nextSibling();
    }  //如果节点不空
    if(!xmlFile.open(QFile::WriteOnly|QFile::Truncate)) //可以用QIODevice，Truncate表示清空原来的内容
        return;
    //输出到文件
    QTextStream out_stream(&xmlFile);
    docXML.save(out_stream,4); //缩进4格
    xmlFile.close();
}

QString ReadWriteXml::getProjectName(){
    return mProjectName;
}
QString ReadWriteXml::getTotalTestNum(){
    return mTotalTestNum;
}
QString ReadWriteXml::getTotalPassNum(){
    return mTotalPassNum;
}
QStringList ReadWriteXml::getProjectList(){
    return mProjectList;
}
QString ReadWriteXml::getHwVersion(){
    return mHwVersion;
}

QString ReadWriteXml::getSwVersion(){
    return mSwVersion;
}

QString ReadWriteXml::get9VNormalMinCur(){
    return m9VNormalMinCur;
}

QString ReadWriteXml::get9VNormalMaxCur(){
    return m9VNormalMaxCur;
}
QString ReadWriteXml::get13VNormalMaxCur(){
    return m13VNormalMaxCur;
}
QString ReadWriteXml::get13VNormalMinCur(){
    return m13VNormalMinCur;
}
QString ReadWriteXml::get15VNormalMinCur(){
    return m15VNormalMinCur;
}
QString ReadWriteXml::get15VNormalMaxCur(){
    return m15VNormalMaxCur;
}
QString ReadWriteXml::get9VSleepMinCur(){
    return m9VSleepMinCur;
}
QString ReadWriteXml::get9VSleepMaxCur(){
    return m9VSleepMaxCur;
}
QString ReadWriteXml::get13VSleepMaxCur(){
    return m13VSleepMaxCur;
}
QString ReadWriteXml::get13VSleepMinCur(){
    return m13VSleepMinCur;
}
QString ReadWriteXml::get15VSleepMinCur(){
    return m15VSleepMinCur;
}
QString ReadWriteXml::get15VSleepMaxCur(){
    return m15VSleepMaxCur;
}
QString ReadWriteXml::get9VIdleMinCur(){
    return m9VIdleMinCur;
}
QString ReadWriteXml::get9VIdleMaxCur(){
    return m9VIdleMaxCur;
}
QString ReadWriteXml::get13VIdleMaxCur(){
    return m13VIdleMaxCur;
}
QString ReadWriteXml::get13VIdleMinCur(){
    return m13VIdleMinCur;
}
QString ReadWriteXml::get15VIdleMinCur(){
    return m15VIdleMinCur;
}
QString ReadWriteXml::get15VIdleMaxCur(){
    return m15VIdleMaxCur;
}
QString ReadWriteXml::getTofDistance(){
    return mTofDistance;
}
QString ReadWriteXml::getNotTofDistance() {
    return mNotTofDistance;
}

QString ReadWriteXml::getProductNum() {
    return mProductNum;
}

QString ReadWriteXml::getFloatVol(){
    return mFloatVol;
}

