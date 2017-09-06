#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ip.h"
#include "thread2.h"
#include "windows.h"

//void autoStart(bool flag)
//{
//    //写入注册表,开机自启动
//    HKEY hKey;
//    //找到系统的启动项
//    LPCTSTR lpRun = (wchar_t*)QString("Software\\Microsoft\\Windows\\CurrentVersion\\Run").utf16();
//    //打开启动项Key
//    long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey);
//    if(lRet == ERROR_SUCCESS)
//    {
//        DWORD dwRet = QApplication::applicationFilePath().length()*2;
//        //添加一个子Key,并设置值
//        if (flag == true)
//        {
//            lRet = RegSetValueEx(hKey, (WCHAR*)QString("test").utf16(), 0, REG_SZ,
//                                 (BYTE*)QApplication::applicationFilePath().utf16(), dwRet);
//        }
//        else
//        {
//            lRet = RegSetValueEx(hKey, (WCHAR*)QString("test").utf16(), 0, REG_SZ,
//                                 (BYTE*)"", dwRet);
//        }
//        //关闭注册表
//        RegCloseKey(hKey);
//        if(lRet != ERROR_SUCCESS)
//        {
//            qDebug("error");
//        }
//    }
//}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    showMainWin = false;

    //QSettings configIniFile("HKEY_LOCAL_MACHINE\\Software\\HIGON\\TC-GUARD-CLIENT", QSettings::NativeFormat);
    //QString iniFileName = configIniFile.value("/inifile").toString();

    //QString application_path2 = QApplication::applicationFilePath();
    //QString application_path = QApplication::applicationDirPath();
    //QString iniFileName = QString("%1/setting.ini").arg(QApplication::applicationDirPath());
    //QSettings configRead(iniFileName, QSettings::IniFormat);
    //HKEY_CURRENT_USER
    QSettings configRead("HKEY_LOCAL_MACHINE\\Software\\HIGON\\TC-GUARD-CLIENT", QSettings::NativeFormat);
    QString masterIP = configRead.value("/IP/MASTER").toString();
    QString gatewayIP = configRead.value("/IP/GATEWAY").toString();
    bool delAndShut = configRead.value("/DELANDSHUT").toBool();

    if(masterIP.isEmpty() && gatewayIP.isEmpty()) {
        showMainWin = true;
    }
    //QString ip = get_localmachine_ip();
    ui->masterIP->setText(masterIP);
    ui->gatewayIP->setText(gatewayIP);
    ui->checkBox->setChecked(delAndShut);

    h1.setHostAddress(masterIP);
    if(masterIP != gatewayIP) {
        hgateway.setHostAddress(gatewayIP);
    }
    hself.setHostAddress(hself.getLocalIP());

    h2.setRemoveFile(delAndShut);
    h2.setShutDown(delAndShut);

    connect(ui->confirm, SIGNAL(clicked(bool)), this, SLOT(slot_setConfigIP()));

    h2.setTcpHandler(&h1);
    h2.setPingGatewayHandler(&hgateway);
    h2.setPingSelfHandler(&hself);

    h1.start();
    hgateway.start();
    hself.start();
    h2.start();

    /*
    QString application_name = QApplication::applicationName();
    //HKEY_LOCAL_MACHINE
    //HKEY_CURRENT_USER
    QSettings *settings1 = new QSettings("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                                         QSettings::NativeFormat);
    QString application_path = QApplication::applicationFilePath();
    settings1->setValue(application_name, application_path.replace("/", "\\"));
    delete settings1;

    autoStart(true);
    */
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setRegEdit(const QString& master, const QString& gateway, const QString& shut)
{
    QSettings configRead("HKEY_LOCAL_MACHINE\\Software\\HIGON\\TC-GUARD-CLIENT", QSettings::NativeFormat);
    QString instdir = configRead.value("/INSTDIR").toString();
    if(instdir.isEmpty()) {
        return;
    }
    //QProcess *cmd = new QProcess;
    QString exefile = instdir + "\\chgregset.exe";
    QString args = "\"" + master + "\" " + "\"" + gateway + "\" " +  "\"" + shut + "\"";
    //cmd->start(strArg);
    //cmd->waitForReadyRead(300);
    //cmd->waitForFinished();

    SHELLEXECUTEINFO sei={sizeof(SHELLEXECUTEINFO)};
    sei.lpVerb = TEXT("runas");
    sei.lpFile = reinterpret_cast<const wchar_t *>(exefile.utf16());//exefile.toStdWString().c_str();//TEXT("C:\\Users\\liaoqiong\\TC-Security-Client\\chgregset.exe");//
    sei.lpParameters = reinterpret_cast<const wchar_t *>(args.utf16());//args.toStdWString().c_str();
    //sei.nShow = SW_SHOW;
    if(!ShellExecuteEx(&sei))
    {
        DWORD dwStatus=GetLastError();
        if(dwStatus==ERROR_CANCELLED)
        {
            qDebug() << "ShellExecuteEx refused";
        }
        else if(dwStatus==ERROR_FILE_NOT_FOUND)
        {
            qDebug() << "ShellExecuteEx not found file";
        }
    }
}

void MainWindow::slot_setConfigIP()
{
    hide();

    h1.mypause();
    hgateway.mypause();
    hself.mypause();

    QString masterIP = ui->masterIP->text();
    QString gatewayIP = ui->gatewayIP->text();
    bool delAndShut = ui->checkBox->checkState() == Qt::Checked;

    h1.setHostAddress(masterIP);
    hgateway.setHostAddress(gatewayIP);

    h2.setRemoveFile(delAndShut);
    h2.setShutDown(delAndShut);

    //QSettings configIniFile("HKEY_LOCAL_MACHINE\\Software\\HIGON\\TC-GUARD-CLIENT", QSettings::NativeFormat);
    //QString iniFileName = configIniFile.value("/inifile").toString();
    //QString iniFileName = QString("%1/setting.ini").arg(QApplication::applicationDirPath());
    //QSettings configWrite(iniFileName, QSettings::IniFormat);
    //QSettings configWrite("HKEY_LOCAL_MACHINE\\Software\\HIGON\\TC-GUARD-CLIENT", QSettings::NativeFormat);
    //configWrite.setValue("/IP/MASTER", masterIP);
    //configWrite.setValue("/IP/GATEWAY", gatewayIP);
    //configWrite.setValue("/DELANDSHUT", delAndShut);

    setRegEdit(masterIP, gatewayIP, delAndShut?"TRUE":"FALSE");

    h1.myresume();
    hgateway.myresume();
    hself.myresume();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    h1.quit();
    h1.wait();
    hgateway.quit();
    hgateway.wait();
    hself.quit();
    hself.wait();
    h2.quit();
    h2.wait();

    QString masterIP = ui->masterIP->text();
    QString gatewayIP = ui->gatewayIP->text();
    bool delAndShut = ui->checkBox->checkState() == Qt::Checked;

    //QSettings configIniFile("HKEY_LOCAL_MACHINE\\Software\\HIGON\\TC-GUARD-CLIENT", QSettings::NativeFormat);
    //QString iniFileName = configIniFile.value("/inifile").toString();
    //QString iniFileName = QString("%1/setting.ini").arg(QApplication::applicationDirPath());
    //QSettings configWrite(iniFileName, QSettings::IniFormat);
    //QSettings configWrite("HKEY_LOCAL_MACHINE\\Software\\HIGON\\TC-GUARD-CLIENT", QSettings::NativeFormat);
    //configWrite.setValue("/IP/MASTER", masterIP);
    //configWrite.setValue("/IP/GATEWAY", gatewayIP);
    //configWrite.setValue("/DELANDSHUT", delAndShut);
    //setRegEdit(masterIP, gatewayIP, delAndShut?"TRUE":"FALSE");

    QMainWindow::closeEvent(event);
}

bool MainWindow::isShowMainWin()
{
    return showMainWin;
}

