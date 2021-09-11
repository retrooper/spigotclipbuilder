#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std::filesystem;
using namespace nlohmann;

//Credit to github.com/lucyy-mc for this script, we will credit her properly soon (contacting her)
const std::string generateJarScript =
        "#!/bin/bash\n"
        "\n"
        "# Generates a patched jar for paperclip\n"
        "\n"
        "if [[ $# < 4 ]]; then\n"
        "    echo \"Usage ./generateJar.sh {input jar} {mojang_jar} {source_url} {name}\"\n"
        "    exit 1;\n"
        "fi;\n"
        "\n"
        "workdir=work/Paperclip\n"
        "\n"
        "mkdir -p $workdir\n"
        "PAPERCLIP_JAR=paperclip.jar\n"
        "\n"
        "if [ ! -f $workdir/$PAPERCLIP_JAR ]; then\n"
        "    if [ ! -d Paperclip ]; then\n"
        "        echo \"Paperclip not found\"\n"
        "        exit 1;\n"
        "    fi\n"
        "    pushd Paperclip\n"
        "    mvn -P '!generate' clean install\n"
        "    if [ ! -f target/paperclip*.jar ]; then\n"
        "        echo \"Couldn't generate paperclip jar\"\n"
        "        exit;\n"
        "    fi;\n"
        "    popd\n"
        "    cp Paperclip/target/paperclip*.jar $workdir/$PAPERCLIP_JAR\n"
        "fi;\n"
        "\n"
        "\n"
        "INPUT_JAR=$1\n"
        "VANILLA_JAR=$2\n"
        "VANILLA_URL=$3\n"
        "NAME=$4\n"
        "\n"
        "which bsdiff 2>&1 >/dev/null\n"
        "if [ $? != 0 ]; then\n"
        "    echo \"Bsdiff not found\"\n"
        "    exit 1;\n"
        "fi;\n"
        "\n"
        "OUTPUT_JAR=$NAME.jar\n"
        "PATCH_FILE=$NAME.patch\n"
        "\n"
        "hash() {\n"
        "    echo $(sha256sum $1 | sed -E \"s/(\\S+).*/\\1/\")\n"
        "}\n"
        "\n"
        "echo \"Computing Patch\"\n"
        "\n"
        "bsdiff $VANILLA_JAR $INPUT_JAR $workdir/$PATCH_FILE\n"
        "\n"
        "genJson() {\n"
        "    PATCH=$1\n"
        "    SOURCE_URL=$2\n"
        "    ORIGINAL_HASH=$3\n"
        "    PATCHED_HASH=$4\n"
        "    echo \"{\"\n"
        "    echo \"    \\\"patch\\\": \\\"$PATCH\\\",\"\n"
        "    echo \"    \\\"sourceUrl\\\": \\\"$SOURCE_URL\\\",\"\n"
        "    echo \"    \\\"originalHash\\\": \\\"$ORIGINAL_HASH\\\",\"\n"
        "    echo \"    \\\"patchedHash\\\": \\\"$PATCHED_HASH\\\"\"\n"
        "    echo \"}\"\n"
        "}\n"
        "\n"
        "\n"
        "echo \"Generating Final Jar\"\n"
        "\n"
        "cp $workdir/$PAPERCLIP_JAR $workdir/$OUTPUT_JAR\n"
        "\n"
        "PATCH_JSON=patch.json\n"
        "\n"
        "genJson $PATCH_FILE $VANILLA_URL $(hash $VANILLA_JAR) $(hash $INPUT_JAR) > $workdir/$PATCH_JSON\n"
        "\n"
        "pushd $workdir\n"
        "\n"
        "jar uf $OUTPUT_JAR $PATCH_FILE $PATCH_JSON\n"
        "\n"
        "popd";

void execute_command(const std::string &command) {
    std::system(command.c_str());
}

void downloadFile(const std::string &url, const std::string &outputName) {
    execute_command(
            "curl --user-agent \"Mozilla/5.0 (X11; Linux x86_64; rv:60.0) Gecko/20100101 Firefox/81.0\" " + url +
            " --output " + outputName);
}

void buildBuildTools(std::string javaBinary, std::string &baseDir, const std::string &version,
                     std::string *buildDataInfoContent) {
    std::string buildToolsBuildDir = baseDir + "/BuildTools-Build";
    bool shouldBuild = true;
    if (shouldBuild) {
        create_directory(buildToolsBuildDir);
        current_path(buildToolsBuildDir);
        std::cout << "Downloading BuildTools..." << std::endl;
        downloadFile(
                "https://hub.spigotmc.org/jenkins/job/BuildTools/lastSuccessfulBuild/artifact/target/BuildTools.jar",
                "BuildTools.jar");
        std::cout << "Building Spigot with BuildTools..." << std::endl;
        execute_command(javaBinary + " -jar BuildTools.jar --rev " + version);
        std::string workPath = buildToolsBuildDir + "/work";
        if (exists(workPath)) {
            //Copy Spigot server jar
            execute_command("cp spigot-" + version + ".jar ..");
            current_path(workPath);
            //Copy vanilla server jar
            execute_command("cp minecraft_server." + version + ".jar ../..");

            std::string buildDataPath = buildToolsBuildDir + "/BuildData";
            if (exists(buildDataPath)) {
                current_path(buildDataPath);
                std::ifstream ifStream("info.json");
                buildDataInfoContent->assign(std::istreambuf_iterator<char>(ifStream),
                                             std::istreambuf_iterator<char>());
                std::cout << "Finished building Spigot with BuildTools..." << std::endl;
            } else {
                std::cerr
                        << "Did not successfully build Spigot with BuildTools, are you sure you have all required components installed?"
                        << std::endl;
                std::exit(-1);
            }
        } else {
            std::cerr
                    << "Failed to build Spigot with BuildTools, are you sure you have all required components installed?"
                    << std::endl;
            std::exit(-1);
        }
    }
}

void buildPaperclip(std::string &baseDir) {
    create_directory("work");
    current_path("work");
    create_directory("Paperclip");
    current_path(baseDir);
    std::string paperClipBuildDir = baseDir + "/Paperclip-Build";
    std::string executableDir = paperClipBuildDir + "/build/libs";
    bool shouldBuild = !exists(executableDir);
    if (shouldBuild) {
        std::cout << "Building PaperClip..." << std::endl;
        create_directory(paperClipBuildDir);
        current_path(paperClipBuildDir);
        execute_command("git clone https://github.com/PaperMC/Paperclip .");
        execute_command("gradle build");
        if (exists(executableDir)) {
            current_path(executableDir);
            //Copy the first file in the directory (there should only be one)
            for (const auto &entry: directory_iterator(executableDir)) {
                //Copy paperclip jar
                execute_command("mv " + entry.path().string() + " paperclip.jar");
                execute_command("cp paperclip.jar ../../..");
                break;
            }
            std::cout << "Finished building PaperClip!" << std::endl;
        } else {
            std::cerr << "Failed to build PaperClip. Are you sure you have all required components installed?"
                      << std::endl;
            std::exit(-1);
        }
    }
}

void patch(const std::string &spigotFileName, const std::string &vanillaFileName, const std::string &vanillaServerURL,
           const std::string &outputFileName) {
    execute_command("bash generateJar.sh " + spigotFileName + " " + vanillaFileName + " " + vanillaServerURL + " " +
                    outputFileName);
}

void build(const std::string &version, const std::string &javaBinary) {
    std::cout << "Starting to build a " << version << " SpigotClip build!" << std::endl;
    std::cout << "This Java Binary will be used to build PaperClip and Spigot: " << javaBinary << std::endl;
    std::string baseDir = current_path().string();
    buildPaperclip(baseDir);
    current_path(baseDir);
    std::string buildDataInfoContent;
    buildBuildTools(javaBinary, baseDir, version, &buildDataInfoContent);
    current_path(baseDir);
    //Actually start patching, we should have everything required to start patching
    json buildDataJson = json::parse(buildDataInfoContent);
    std::string vanillaServerURL = buildDataJson.value("serverUrl", "invalid");

    //Generate patch script
    std::ofstream generateJarFile("generateJar.sh");
    generateJarFile << generateJarScript;
    generateJarFile.close();

    //Patch with script
    std::string spigotFileName = "spigot-" + version + ".jar";
    std::string vanillaFileName = "minecraft_server." + version + ".jar";
    std::string outputFileName = "spigotclip-" + version + ".jar";

    execute_command("cp paperclip.jar work/Paperclip");
    patch(spigotFileName, vanillaFileName, vanillaServerURL, "spigotclip-" + version);
    create_directory("output");
    current_path("work/Paperclip");
    execute_command("cp " + outputFileName + " ../../output");
    std::cout << "Finished patching " << outputFileName << std::endl;
    std::cout << "Enjoy :D" << std::endl;
}

int main() {
    //TODO Document Requirements: Maven, Gradle, Git, bsdiff, Java 16

    //TODO build paperclip and buildtools in parallel if possible
    build("1.17.1", "/usr/lib/jvm/java-16-openjdk/bin/java");
    return 0;
}