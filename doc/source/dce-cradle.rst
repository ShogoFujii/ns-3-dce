.. include:: replace.txt

|dce_cradle|
============

This document describes what |dce_cradle| is, how we can use it, how we extend it.

Tutorials and how to reproduce the experiment of WNS3 2013 paper is available :doc:`dce-cradle-usecase`.

What is |dce_cradle|?
---------------------
|dce_cradle| enables us to use Linux kernel via Direct Code Execution from the ns-3 native socket application. Applications can access it via ns-3 socket API. Currently (20th Nov. 2012) the following sockets are available:

 * IPv4 UDP
 * IPv4 TCP
 * IPv4 RAW socket
 * IPv4 DCCP


Installing |dce_cradle|
-----------------------
|dce_cradle| is already integrated in ns-3-dce module. You can just build and install DCE as instructed in the parent document.

How to use it
-------------

::

  OnOffHelper onoff = OnOffHelper ("ns3::LinuxTcpSocketFactory",
                                   InetSocketAddress (interfaces.GetAddress (1), 9));


How to extend it
----------------
(To be added)

Article
-------
The project originally started during `GSoC project 2012 <http://www.nsnam.org/wiki/index.php/GSOC2012Projects#Allow_ns-3_native_applications_to_use_the_ns-3-linux_linux_kernel_stack>`_

the paper (to be added if it will be published)

