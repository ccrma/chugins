#include "../host.h"
#include "../vst3Ctx.h"

#include <iostream>
#include <fstream>
#include <iomanip> // std::put_time
#include <ctime>

// Produces a yaml dump of the requested plugin on stdout
// See important comments in host.h regarding proper handling 
// of multi-threaded plugins.
class Dumpit : public VST3Host
{
public:
    Dumpit() 
    {
        s_unmanagedHost = this;
    }
    ~Dumpit() 
    {
    }

    int DumpOne(char const *path, std::ostream &ostream)
    {
        std::string modpath(path);
        this->OpenPlugin(modpath, [this, &ostream](VST3Ctx *ctx) /* control capture by value or reference */
        {
            if(ctx)
            {
                // we *are* the message thread so must pop the queue 
                // NB: pluginInstance::restartComponent *must* Delegate
                // a single method.
                if(this->m_msgQueue.Size())
                {
                    auto item = this->m_msgQueue.Pop();
                    item();
                }
                else
                {
                    // std::cerr << "No component restart\n";
                }
                ctx->Print(ostream, true/*detailed*/);
                delete ctx;
            }
        });
        return 0;
    }

    void onPluginLoaded(std::ostream &ostream, VST3Ctx *ctx)
    {
        if(ctx)
        {
            ctx->Print(ostream, true/*detailed*/);
            delete ctx;
        };
    }

};

static void
usage(char const *appnm)
{
    std::cout << "dumpVST3 usage:\n"
        << "   (noargs): prints all known plugins (to stdout)\n"
        << "   <pluginfile.vst3>: dumps yaml overview of a single .vstfile (to stdout)\n"
        << "  -dstdir <dirname>: writes a new .yaml file for each plugin into dstdir\n"
        << "  -h: prints this message and exits\n"
    ;
}

int main(int argc, char* argv[])
{
    Dumpit app;
    std::string vstfile;
    std::string dstdir;
    for(int i=1;i<argc;i++)
    {
        if(!strncmp(argv[i], "-h", 2) || !strncmp(argv[i], "--", 2))
        {
            usage(argv[0]);
            return 1;
        }
        else
        if(!strcmp(argv[i], "-dstdir"))
        {
            if(++i < argc)
                dstdir = argv[i];
            else
            {
                usage(argv[0]);
                return 2;
            }
        }
        else
            vstfile = argv[i];
    }
    if(vstfile.size() == 0 && dstdir.size() == 0)
        app.PrintAllInstalledPlugins(std::cout);
    else
    if(vstfile.size() > 0)
        return app.DumpOne(vstfile.c_str(), std::cout);
    else
    {
        std::vector<std::string> plugins;
        int err = app.GetKnownPlugins(plugins);
        if(!err)
        {
            for(int i=0;i<plugins.size();i++)
            {
                std::string yamlfile = dstdir;
                std::string const &vst3file = plugins[i];
                std::size_t found = vst3file.find_last_of("/\\");
                if(found != std::string::npos)
                    yamlfile.append(vst3file.substr(found));
                else
                {
                    yamlfile.append("/");
                    yamlfile.append(vst3file);
                }
                found = yamlfile.find_last_of(".");
                if(found != std::string::npos)
                    yamlfile = yamlfile.substr(0, found);
                yamlfile.append(".yaml");
                std::filebuf fb;
                if(fb.open(yamlfile, std::ios::out))
                {
                    std::time_t t = std::time(nullptr);
                    std::tm *tm = std::localtime(&t);
                    std::ostream ostr(&fb);
                    ostr << "# This file was written by dumpVST3 on "
                         << std::put_time(tm, "%c") << "\n";
                    ostr << "# Each time you re-scan plugins it may be overwritten.\n";
                    ostr << "# Edit at your own risk.\n";
                    app.DumpOne(vst3file.c_str(), ostr);
                    fb.close();
                    std::cout << yamlfile << "\n";
                }
                else
                    std::cerr << "problem opening " << yamlfile << " for writing\n";
            }
        }
        else
            std::cerr << "No VST3 plugins found!\n";
    }
    // _CrtDumpMemoryLeaks();
}
