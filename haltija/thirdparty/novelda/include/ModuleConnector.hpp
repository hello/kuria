#ifndef MODULECONNECTOR_HPP
#define MODULECONNECTOR_HPP

#include "Bytes.hpp"

#include <string>
#include <memory>
#include <iostream>

//struct RadarInterface;

namespace XeThru {

class X2M200;
class XEP;
class X4M300;
class X2;
class NotSupported;
class Transport;
class AbstractLoggerIo;
class DataRecorder;
class ModuleConnectorImpl;

/**
 * @class ModuleConnector
 *
 * This class is responsible for establishing contact with the Xethru module.
 * Use one of the get_XXX to access the modules in various ways.
 *
 */
class ModuleConnector
{
public:

    /**
     * Constructor
     *
     * The constructor will open the device file or COM port given.
     *
     * @param device_name Name of the device file for example /dev/ttyACM0 or COM4
     * @param log_level     The log level to use during operation
     * @param logger_io     The logging io implementation to use
     *
     */
    ModuleConnector(
        const std::string & device_name,
        int log_level,
        AbstractLoggerIo * logger_io);

    /**
     * Constructor
     *
     * The constructor will open the device file or COM port given.
     *
     * @param a_device_name Name of the device file for example /dev/ttyACM0 or COM4
     *
     */
    ModuleConnector(const std::string & device_name);

    /**
     * Constructor
     *
     * The constructor will open the device file or COM port given.
     *
     * @param a_device_name Name of the device file for example /dev/ttyACM0 or COM4
     * @param log_level     The log level to use during operation
     *
     */
    ModuleConnector(const std::string & device_name, int log_level);

    /**
     * Destructor
     *
     * It invoces close() and cleans up all resources in use.
     * It will no longer be possible to use this object or any of its interfaces.
     *
     */
    virtual ~ModuleConnector();

    /**
     * Opens a new connection to a module via some serial device
     *
     * @param device Name of the device file for example /dev/ttyACM0 or COM4
     */
    int open(const std::string device_name);

    /**
     * Close an open connection to the module
     *
     * @return 0 on success, otherwise return 1
     */
    void close();

    /**
     * Provides the git sha of the ModuleConnector repository
     *
     */
    std::string git_sha();

    /**
     * Set log level during ModuleConnector operation
     *
     * @param new_log_level The new log level to use
     */
    void set_log_level(int new_log_level);

    /**
     * Set default timeout for commands sent to the module.
     *
     * @param new_default_timeout
     */
    void set_default_timeout(unsigned int new_default_timeout);

    /**
     * Returns a reference to the DataRecorder application interface
     * @return DataRecorder The reference to the DataRecorder interface
     * @see DataRecorder
     */
    DataRecorder & get_data_recorder();

    /**
     * Not supported
     * @return Not supported
     */
    Transport & get_transport();

    /**
     * Returns a reference to the X2M200 module application interface
     * @return X2M200 The reference to the X2M200 interface
     */
    X2M200 & get_x2m200();

    /**
     * Not supported
     * @return Not supported
     */
    X2 & get_x2();

    /**
     * Returns a reference to the low level XEP interface.
     *
     * @return The reference to the XEP interface.
     *
     */
    XEP & get_xep();

    /**
     * Returns a reference to the X4M300 module application interface.
     *
     * @return The reference to the X4M300 interface.
     *
     */
    X4M300 & get_x4m300();

    /**
     * Not supported
     * @return Not supported
     */
    NotSupported & get_not_supported();

private:
    std::unique_ptr<ModuleConnectorImpl> pimpl;
};

} // namespace

#endif
