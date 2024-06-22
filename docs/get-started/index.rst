.. _getting_started:

Getting started
===============

Getting started may be the most challenging part of every new library.
This guide is describing how to start with the library quickly and effectively

.. _download_library:

Download library
^^^^^^^^^^^^^^^^

Library is primarly hosted on `Github <https://github.com/MaJerle/lwcell>`_.

You can get it by:

* Downloading latest release from `releases area <https://github.com/MaJerle/lwcell/releases>`_ on Github
* Cloning ``main`` branch for latest stable version
* Cloning ``develop`` branch for latest development

Download from releases
**********************

All releases are available on Github `releases area <https://github.com/MaJerle/lwcell/releases>`_.

Clone from Github
*****************

First-time clone
""""""""""""""""

This is used when you do not have yet local copy on your machine.

* Make sure ``git`` is installed.
* Open console and navigate to path in the system to clone repository to. Use command ``cd your_path``
* Clone repository with one of available options below

  * Run ``git clone --recurse-submodules https://github.com/MaJerle/lwcell`` command to clone entire repository, including submodules
  * Run ``git clone --recurse-submodules --branch develop https://github.com/MaJerle/lwcell`` to clone `development` branch, including submodules
  * Run ``git clone --recurse-submodules --branch main https://github.com/MaJerle/lwcell`` to clone `latest stable` branch, including submodules

* Navigate to ``examples`` directory and run favourite example

Update cloned to latest version
"""""""""""""""""""""""""""""""

* Open console and navigate to path in the system where your repository is located. Use command ``cd your_path``
* Run ``git pull origin main`` command to get latest changes on ``main`` branch
* Run ``git pull origin develop`` command to get latest changes on ``develop`` branch
* Run ``git submodule update --init --remote`` to update submodules to latest version

.. note::
  This is preferred option to use when you want to evaluate library and run prepared examples.
  Repository consists of multiple submodules which can be automatically downloaded when cloning and pulling changes from root repository.

Add library to project
^^^^^^^^^^^^^^^^^^^^^^

At this point it is assumed that you have successfully download library, either with ``git clone`` command or with manual download from the library releases page.
Next step is to add the library to the project, by means of source files to compiler inputs and header files in search path.

*CMake* is the main supported build system. Package comes with the ``CMakeLists.txt`` and ``library.cmake`` files, both located in the ``lwcell`` directory:

* ``library.cmake``: It is a fully configured set of variables and with library definition. User can include this file to the project file with ``include(path/to/library.cmake)`` and then manually use the variables provided by the file, such as list of source files, include paths or necessary compiler definitions. It is up to the user to properly use the this file on its own.
* ``CMakeLists.txt``: It is a wrapper-only file and includes ``library.cmake`` file. It is used for when user wants to include the library to the main project by simply calling *CMake* ``add_subdirectory`` command, followed by ``target_link_libraries`` to link external library to the final project.

.. tip::
    Open ``library.cmake`` and analyze the provided information. Among variables, you can also find list of all possible exposed libraries for the user.

If you do not use the *CMake*, you can do the following:

* Copy ``lwcell`` folder to your project, it contains library files
* Add ``lwcell/src/include`` folder to `include path` of your toolchain. This is where `C/C++` compiler can find the files during compilation process. Usually using ``-I`` flag
* Add source files from ``lwcell/src/`` folder to toolchain build. These files are built by `C/C++` compiler
* Copy ``lwcell/src/include/lwcell/lwcell_opts_template.h`` to project folder and rename it to ``lwcell_opts.h``
* Build the project

Configuration file
^^^^^^^^^^^^^^^^^^

Configuration file is used to overwrite default settings defined for the essential use case.
Library comes with template config file, which can be modified according to the application needs.
and it should be copied (or simply renamed in-place) and named ``lwcell_opts.h``

.. note::
    Default configuration template file location: ``lwcell/src/include/lwcell/lwcell_opts_template.h``.
    File must be renamed to ``lwcell_opts.h`` first and then copied to the project directory where compiler
    include paths have access to it by using ``#include "lwcell_opts.h"``.

.. tip::
    If you are using *CMake* build system, define the variable ``LWCELL_OPTS_FILE`` before adding library's directory to the *CMake* project.
    Variable must contain the path to the user options file. If not provided and to avoid build error, one will be generated in the build directory.

Configuration options list is available available in the :ref:`api_lwcell_opt` section.
If any option is about to be modified, it should be done in configuration file

.. literalinclude:: ../../lwcell/src/include/lwcell/lwcell_opts_template.h
    :language: c
    :linenos:
    :caption: Template configuration file

.. note::
    If you prefer to avoid using configuration file, application must define
    a global symbol ``LWCELL_IGNORE_USER_OPTS``, visible across entire application.
    This can be achieved with ``-D`` compiler option.
