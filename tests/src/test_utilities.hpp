#pragma once

/////////////////////////////////////////////////////////
#include <cstdint>
#include <limits>
#include <memory>
#include <sys/wait.h>
/////////////////////////////////////////////////////////
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
/////////////////////////////////////////////////////////
#include <callbacks.hpp>
#include <contexts.hpp>
#include <moqt.hpp>
#include <subscription_builder.hpp>
#include <utilities.hpp>
/////////////////////////////////////////////////////////

static const char* Target = "127.0.0.1";
const std::uint16_t serverPort = 4567;

static const char* CertFile = RAVEN_CERT_FILE_PATH;
static const char* KeyFile = RAVEN_KEY_FILE_PATH;


static inline std::unique_ptr<rvn::MOQTClient> client_setup()
{
    std::unique_ptr<rvn::MOQTClient> moqtClient = std::make_unique<rvn::MOQTClient>();

    QUIC_REGISTRATION_CONFIG RegConfig = { "test1", QUIC_EXECUTION_PROFILE_LOW_LATENCY };
    moqtClient->set_regConfig(&RegConfig);

    QUIC_SETTINGS Settings;
    std::memset(&Settings, 0, sizeof(Settings));
    Settings.IdleTimeoutMs = 0;
    Settings.IsSet.IdleTimeoutMs = TRUE;
    Settings.PeerUnidiStreamCount = (std::numeric_limits<std::uint16_t>::max());
    Settings.IsSet.PeerUnidiStreamCount = TRUE;
    Settings.IsSet.StreamMultiReceiveEnabled = TRUE;
    Settings.StreamMultiReceiveEnabled = TRUE;
    moqtClient->set_Settings(&Settings, sizeof(Settings));

    QUIC_CREDENTIAL_CONFIG credConfig;
    std::memset(&credConfig, 0, sizeof(credConfig));
    credConfig.Type = QUIC_CREDENTIAL_TYPE_NONE;
    credConfig.Flags =
    QUIC_CREDENTIAL_FLAG_CLIENT | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;
    moqtClient->set_CredConfig(&credConfig);

    QUIC_BUFFER AlpnBuffer = { sizeof("test1") - 1, (uint8_t*)"test1" };
    moqtClient->set_AlpnBuffers(&AlpnBuffer);
    moqtClient->set_AlpnBufferCount(1);

    moqtClient->set_connectionCb(rvn::callbacks::client_connection_callback);
    moqtClient->set_listenerCb(rvn::callbacks::client_listener_callback);
    moqtClient->set_controlStreamCb(rvn::callbacks::client_control_stream_callback);
    moqtClient->set_dataStreamCb(rvn::callbacks::client_data_stream_callback);

    moqtClient->start_connection(QUIC_ADDRESS_FAMILY_UNSPEC, Target, serverPort);

    return moqtClient;
}


static inline std::unique_ptr<rvn::MOQTServer> server_setup()
{
    auto dm = std::make_shared<rvn::DataManager>();
    std::unique_ptr<rvn::MOQTServer> moqtServer = std::make_unique<rvn::MOQTServer>(dm);

    QUIC_REGISTRATION_CONFIG RegConfig = { "test1", QUIC_EXECUTION_PROFILE_LOW_LATENCY };
    moqtServer->set_regConfig(&RegConfig);

    moqtServer->set_listenerCb(rvn::callbacks::server_listener_callback);
    moqtServer->set_connectionCb(rvn::callbacks::server_connection_callback);
    moqtServer->set_controlStreamCb(rvn::callbacks::server_control_stream_callback);
    moqtServer->set_dataStreamCb(rvn::callbacks::server_data_stream_callback);

    QUIC_BUFFER AlpnBuffer = { sizeof("test1") - 1, (uint8_t*)"test1" };
    moqtServer->set_AlpnBuffers(&AlpnBuffer);
    moqtServer->set_AlpnBufferCount(1);

    const uint64_t IdleTimeoutMs = 0;
    QUIC_SETTINGS Settings;
    std::memset(&Settings, 0, sizeof(Settings));
    Settings.IdleTimeoutMs = IdleTimeoutMs;
    Settings.IsSet.IdleTimeoutMs = TRUE;
    Settings.PeerBidiStreamCount = 1;
    Settings.IsSet.PeerBidiStreamCount = TRUE;
    Settings.IsSet.StreamMultiReceiveEnabled = TRUE;
    Settings.StreamMultiReceiveEnabled = TRUE;
    moqtServer->set_Settings(&Settings, sizeof(Settings));

    // certificates
    QUIC_CERTIFICATE_FILE certFile;
    certFile.CertificateFile = (char*)CertFile;
    certFile.PrivateKeyFile = (char*)KeyFile;
    // setting up credential configuration
    QUIC_CREDENTIAL_CONFIG credConfig;
    std::memset(&credConfig, 0, sizeof(credConfig));
    credConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
    credConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    credConfig.CertificateFile = &certFile;
    moqtServer->set_CredConfig(&credConfig);

    QUIC_ADDR Address;
    std::memset(&Address, 0, sizeof(Address));
    QuicAddrSetFamily(&Address, QUIC_ADDRESS_FAMILY_UNSPEC);
    QuicAddrSetPort(&Address, serverPort);

    moqtServer->start_listener(&Address);

    return moqtServer;
}
