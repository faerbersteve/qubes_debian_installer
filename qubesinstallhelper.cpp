#include "qubesinstallhelper.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;


inline bool file_exists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

qubesInstallHelper::qubesInstallHelper(std::vector<qubesPkg *> p,std::string f)
{
    packages=p;
    folder=f;
}

void qubesInstallHelper::install()
{
    int ret=0;
    bool hasError{false};

    if (!file_exists(folder))
    {
        cout << "Couldn't find folder with deb packages" << endl;
        return;
    }

    chdir(folder.c_str());

    //prepare host
    cout << "Prepare host.." << endl;

    runCmd("hostnamectl set-hostname dom0");

    runCmd("mkdir -p /run/qubes");
    runCmd("chmod 777 -R /run/qubes");
    runCmd("mkdir -p /var/log/qubes");
    runCmd("chmod 777 -R /var/log/qubes");

    //check for xen

    //activate xen
    cout << "Activate xen..." << endl;

    runCmd("modprobe xen-gntalloc");
    runCmd("modprobe xen-gntdev");
    runCmd("modprobe xen-evtchn");

    runCmd("chmod 777 -R /dev/xen");

    writeModuleLoadConf();

    //start install
    cout << "Start installation.." << endl;

    for(auto p : packages)
    {
        ret=p->readVersion();

        if (ret!=0)
        {
            cout << "Error while reading version of project " << p->projectName << endl;
            hasError=true;
            break;
        }

        cout << "Install project-packages " << p->projectName << endl;

        p->installPackages(true);

        if (ret!=0)
        {
            cout << "Error while installing project " << p->projectName << endl;
            hasError=true;
            break;
        }

        ret=installMissing();

        if (ret!=0)
        {
            cout << "Error while fixing apt, installing project " << p->projectName << endl;
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
        cout << "Installation failed with error" << endl;
    }
}

qubesPkg *qubesInstallHelper::getProjPackage(string name)
{
    for(auto q: packages)
    {
        if (q->projectName==name)
            return q;
    }
    return nullptr;
}

int qubesInstallHelper::installMissing()
{
    return runCmd("apt install -y -f");
}

int qubesInstallHelper::runCmd(string cmd)
{
    return system(cmd.c_str());
}

void qubesInstallHelper::writeModuleLoadConf()
{
    fstream of("/usr/lib/modules-load.d/xen.conf",std::fstream::out);

    of << "xen-evtchn" << endl;
    of << "xen-gntdev" << endl;
    of << "xen-gntalloc" << endl;
    of << "xen-blkback" << endl;
    of << "xen-pciback" << endl;
    of << "xen-privcmd" << endl;
    of << "xen-acpi-processor" << endl;

    of.close();
}
