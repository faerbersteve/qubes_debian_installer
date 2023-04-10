#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include "qubespkg.h"
#include "debiancontrolfile.h"
#include "qubesinstallhelper.h"

using namespace std;

std::vector<qubesPkg*> getPackages()
{
    std::vector<qubesPkg*> packages{};
    qubesPkg* qPack=nullptr;

    //load package list
    qPack=new qubesPkg("qubes-core-vchan-xen");
    qPack->addPackageName("libvchan-xen", PkgInstallFlag::ALL);
    qPack->addPackageName("libvchan-xen-dev", PkgInstallFlag::FOR_DEV);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-core-qubesdb");
    qPack->addPackageName("qubesdb-dev", PkgInstallFlag::FOR_DEV);
    qPack->addPackageName("libqubesdb", PkgInstallFlag::ALL);
    qPack->addPackageName("python3-qubesdb", PkgInstallFlag::ALL);
    qPack->addPackageName("qubesdb", PkgInstallFlag::ALL);
    qPack->addPackageName("qubesdb-dom0", PkgInstallFlag::FOR_PROD);
    qPack->addPackageName("qubesdb-vm", PkgInstallFlag::IGNORE);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-core-qrexec");
    qPack->addPackageName("libqrexec-utils2", PkgInstallFlag::ALL);
    qPack->addPackageName("libqrexec-utils-dev", PkgInstallFlag::FOR_DEV);
    qPack->addPackageName("qubes-core-qrexec", PkgInstallFlag::FOR_PROD);
    qPack->addPackageName("qubes-core-qrexec-dom0", PkgInstallFlag::FOR_PROD);
    qPack->addPackageName("python3-qrexec", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-rpm-oxide");
    qPack->addPackageName("qubes-rpm-oxide", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-linux-utils");
    qPack->addPackageName("python3-qubesimgconverter", PkgInstallFlag::ALL);
    qPack->addPackageName("qubes-utils", PkgInstallFlag::FOR_PROD);
    qPack->addPackageName("qubes-kernel-vm-support", PkgInstallFlag::IGNORE);
    qPack->addPackageName("libqubes-rpc-filecopy2", PkgInstallFlag::ALL);
    qPack->addPackageName("libqubes-rpc-filecopy-dev", PkgInstallFlag::FOR_DEV);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-linux-kernel");
    qPack->addPackageName("kernel-latest-qubes-vm", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-core-agent-linux");
    qPack->addPackageName("qubes-core-agent-dom0-updates", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-core-admin"); //no debian files in qubes github repo
    qPack->addPackageName("qubes-core-dom0", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-repo-templates");
    qPack->addPackageName("qubes-repo-templates", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-gui-common");
    qPack->addPackageName("qubes-gui-common", PkgInstallFlag::ALL);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-gui-daemon");
    qPack->addPackageName("qubes-gui-daemon", PkgInstallFlag::FOR_PROD);
    qPack->addPackageName("qubes-gui-daemon-pulseaudio", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-gui-agent-linux"); //issue with missing Trolltech.conf
    qPack->addPackageName("qubes-gui-agent", PkgInstallFlag::IGNORE);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-core-admin-client"); //sphinxdoc throws error
    qPack->addPackageName("python3-qubesadmin", PkgInstallFlag::FOR_PROD);
    qPack->addPackageName("qubes-core-admin-client", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-app-linux-img-converter");
    qPack->addPackageName("qubes-img-converter", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-artwork"); //needs dependency qubesimgconverter
    qPack->addPackageName("qubes-artwork", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-python-qasync");
    qPack->addPackageName("python3-qasync", PkgInstallFlag::ALL);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-desktop-linux-common");
    qPack->addPackageName("qubes-desktop-linux-common", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    qPack=new qubesPkg("qubes-manager"); //has qubes-python-qasync dependency
    qPack->addPackageName("qubes-manager", PkgInstallFlag::FOR_PROD);

    packages.push_back(qPack);

    return packages;
}

int main(int argc, char** argv)
{
    std::vector<qubesPkg*> packages{};
    qubesInstallHelper* helper=nullptr;
    std::string arg{};
    std::string folderName{"output"};
    int ret{0};
    bool hasError{false};
    bool createPackages{false};
    bool installQubes{false};
    bool showHelp{true};
    bool noRoot{false};
    bool debug{false};

    cout << "---Qubes debian/ubuntu package creator & installer---" << endl;

    if (argc<=1)
        showHelp=true;
    else
    {
        for(int i=1; i <argc; ++i)
        {
            arg=argv[i];

            if (arg=="-create")
            {
                createPackages=true;
                showHelp=false;
            }
            else if (arg=="-install")
            {
                installQubes=true;
                showHelp=false;
            }
            else if (arg.length()>8 && arg.substr(0,8)=="-folder=")
            {
              folderName=arg.substr(8);
            }
            else if (arg=="-noroot")
            {
                noRoot=true;
            }
            else if (arg=="-help")
            {
                showHelp=true;
                break;
            }
            else if (arg=="-debug")
            {
                debug=true;
            }
        }
    }

    if (showHelp)
    {
        cout << "Possible args:" << endl;
        cout << "-create        create the debian packages" << endl;
        cout << "-install       install qubes with the debian packages" << endl;
        cout << "-folder=[NAME] set the folder name, default is output" << endl;
        cout << "-noroot        ignore missing root privileges" << endl;

        return 0;
    }

    if (!noRoot && getuid()!=0)
    {
        cout << "This app needs to run as root." << endl;
        return -2;
    }

    packages=getPackages();

    if (createPackages)
    {
        cout << "deb files will be created in the output folder" << endl;

        cout << "starting with downloading and creating deb packages..." << endl;

        qubesPkg::outputFolder=folderName;
        qubesPkg::initForCreate();

        for(auto p :packages)
        {
            ret=p->download();

            if (ret!=0)
            {
                cout << "Error while downloading project " << p->projectName << endl;
                hasError=true;
                break;
            }

            ret=p->createPackage();

            if (ret!=0)
            {
                cout << "Error while create package of project " << p->projectName << endl;
                hasError=true;
                break;
            }

            ret=p->installPackages(false);

            if (ret!=0)
            {
                cout << "Error while install package(s) of project " << p->projectName << endl;
                hasError=true;
                break;
            }

            if (debug)
            {
                cout << "press key to continue with next package"<<endl;

                cin.get();
            }
        }

        if (hasError)
        {
            getchar();
            return ret;
        }

        cout << "Clean up" << endl;

        for(auto p :packages)
        {
            p->cleanUp();
        }

        cout << "Created all packages, ready for install" << endl;
    }

    if (installQubes)
    {
        cout << "starting with installation..." << endl;

        helper=new qubesInstallHelper(packages);

        helper->debug=debug;
        helper->folder=folderName;
        helper->install();

        cout << endl << "installation finished, please reboot" << endl;
    }
    else
    {
        cout << endl << "reached end of programm..." << endl;
    }

    getchar();

    return 0;
}
