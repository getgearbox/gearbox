#include <gearbox/core/logger.h>
#include <gearbox/core/util.h>

int do_system (const std::string & cmd) {
    int run = 0;
    run += cmd.find("./mkrsrc") == 0;
    run += cmd.find("cp -p") == 0;
    run += cmd.find("qemu-img") == 0;
    run += cmd.find("dd bs=") == 0;
    run += cmd.find("mkswap") == 0;
    run += cmd.find("tar") == 0;
    run += cmd.find("diff") == 0;
    run += cmd.find("../../node/scripts/gen_pv_libvirt_xml") == 0;
    run += cmd.find("../../node/scripts/gen_kvm_libvirt_xml") == 0;
    run += cmd.find("../scripts/gen_dummy_libvirt_xml") == 0;
    run += cmd.find("../../node/scripts/get_xen_info") == 0;
    run += cmd.find("../../node/scripts/gen_hvm_libvirt_xml") == 0;
    run += cmd.find("../../node/scripts/gen_dummy_libvirt_xml") == 0;
    run += cmd.find("./addnode") == 0;
    run += cmd.find("./mkdb") == 0;
    run += cmd.find("/dev/null") == 0; // for testing error cases
    run += cmd.find("yinst set yca_client_certs.use_proxy") == 0;

    if( run ) {
        return system(cmd.c_str());
    }
    else {
        _DEBUG("Skipping the run: " << cmd << " per " << __FILE__);
    }
    return 0;
}
