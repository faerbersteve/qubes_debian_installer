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


    if (!file_exists(folder))
    {
        cout << "Couldn't find folder with deb packages" << endl;
        return;
    }

    chdir(folder.c_str());

    //prepare host
    cout << "Prepare host.." << endl;

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

    //start install
    cout << "Start installation.." << endl;

    for(auto p : packages)
    {
        ret=p->readVersion();

        if (ret!=0)
        {
            cout << "Error while reading version of project " << p->projectName << endl;
            break;
        }

        cout << "Install project-packages " << p->projectName << endl;

        p->installPackages(true);

        if (ret!=0)
        {
            cout << "Error while installing project " << p->projectName << endl;
            break;
        }

        installMissing();

        cin.get();
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
