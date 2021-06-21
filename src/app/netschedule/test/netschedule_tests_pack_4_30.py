#
# Authors: Sergey Satskiy
#
# $Id: netschedule_tests_pack_4_30.py 543877 2017-08-15 13:30:30Z satskyse $
#

"""
Netschedule server tests pack for the features appeared in NS-4.30.1 and up
"""

from netschedule_tests_pack import TestBase
from netschedule_tests_pack_4_10 import execAny

from cgi import parse_qs
import socket
import time


class Scenario2000(TestBase):

    """Scenario 2000"""

    def __init__(self, netschedule):
        TestBase.__init__(self, netschedule)

    @staticmethod
    def getScenario():
        """Provides the scenario"""
        return "SUBMIT into a virtual scope, " \
               "GET2 with matching virtual scope"

    def execute(self):
        """Should return True if the execution completed successfully"""
        self.fromScratch()

        ns_client = self.getNetScheduleService( 'TEST', 'scenario2000' )
        ns_client.set_client_identification( 'node', 'session' )

        execAny(ns_client, 'SETSCOPE scope=WN_SCOPE:::test_wn')
        execAny(ns_client, 'SUBMIT blah')

        wn_client = self.getNetScheduleService('TEST', 'scenario2000')
        wn_client.set_client_identification('test_wn', 'session')

        output = execAny(wn_client, 'GET2 wnode_aff=0 any_aff=1')
        if output == '':
            raise Exception("Expected a job, got nothing")

        values = parse_qs(output, True, True)
        receivedJobID = values['job_key'][0]
        if not receivedJobID:
            raise Exception("Expected a job, got: " + output)
        return True


class Scenario2001(TestBase):

    """Scenario 2001"""

    def __init__(self, netschedule):
        TestBase.__init__(self, netschedule)

    @staticmethod
    def getScenario():
        """Provides the scenario"""
        return "SUBMIT into a virtual scope, " \
               "GET2 with non matching virtual scope"

    def execute(self):
        """Should return True if the execution completed successfully"""
        self.fromScratch()

        ns_client = self.getNetScheduleService( 'TEST', 'scenario2001' )
        ns_client.set_client_identification( 'node', 'session' )

        execAny(ns_client, 'SETSCOPE scope=WN_SCOPE:::test_wn')
        execAny(ns_client, 'SUBMIT blah')

        wn_client = self.getNetScheduleService('TEST', 'scenario2001')
        wn_client.set_client_identification('test_not_matching_wn', 'session')

        output = execAny(wn_client, 'GET2 wnode_aff=0 any_aff=1')
        if output != '':
            raise Exception("Expected no job, got: " + output)
        return True


class Scenario2002(TestBase):

    """Scenario 2002"""

    def __init__(self, netschedule):
        TestBase.__init__(self, netschedule)

    @staticmethod
    def getScenario():
        """Provides the scenario"""
        return "GET2 with timeout and port; " \
               "SUBMIT to a matching virtual scope; " \
               "Notifications expected"

    def execute(self):
        """Should return True if the execution completed successfully"""
        self.fromScratch()

        notifSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        notifSocket.bind(("", 0))
        notifPort = notifSocket.getsockname()[1]

        wn_client = self.getNetScheduleService('TEST', 'scenario2002')
        wn_client.set_client_identification('test_wn', 'session')

        # Get a job with a timeout
        execAny(wn_client,
                'GET2 wnode_aff=0 any_aff=1 exclusive_new_aff=0 port=' +
                str(notifPort) + ' timeout=5')

        # Submit a job to a matching virtual scope
        ns_client = self.getNetScheduleService( 'TEST', 'scenario2002' )
        ns_client.set_client_identification( 'node', 'session' )

        execAny(ns_client, 'SETSCOPE scope=WN_SCOPE:::test_wn')
        execAny(ns_client, 'SUBMIT blah')

        # Check that there were no notifications
        time.sleep(3)
        result = self.getNotif(notifSocket)
        notifSocket.close()
        if result != 1:
            raise Exception("Expect notifications but received none")
        return True

    def getNotif(self, s):
        """Retrieves notifications"""
        try:
            data = s.recv(8192, socket.MSG_DONTWAIT)
            if "queue=TEST" not in data:
                raise Exception("Unexpected notification in socket")
            return 1
        except Exception, ex:
            if "Unexpected notification in socket" in str(ex):
                raise
        return 0


class Scenario2003(TestBase):

    """Scenario 2003"""

    def __init__(self, netschedule):
        TestBase.__init__(self, netschedule)

    @staticmethod
    def getScenario():
        """Provides the scenario"""
        return "GET2 with timeout and port; " \
               "SUBMIT to a non-matching virtual scope; " \
               "No notifications expected"

    def execute(self):
        """Should return True if the execution completed successfully"""
        self.fromScratch()

        notifSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        notifSocket.bind(("", 0))
        notifPort = notifSocket.getsockname()[1]

        wn_client = self.getNetScheduleService('TEST', 'scenario2003')
        wn_client.set_client_identification('test_wn', 'session')

        # Get a job with a timeout
        execAny(wn_client,
                'GET2 wnode_aff=0 any_aff=1 exclusive_new_aff=0 port=' +
                str(notifPort) + ' timeout=5')

        # Submit a job to a matching virtual scope
        ns_client = self.getNetScheduleService( 'TEST', 'scenario2003' )
        ns_client.set_client_identification( 'node', 'session' )

        execAny(ns_client, 'SETSCOPE scope=WN_SCOPE:::non_matching_test_wn')
        execAny(ns_client, 'SUBMIT blah')

        # Check that there were no notifications
        time.sleep(3)
        result = self.getNotif(notifSocket)
        notifSocket.close()
        if result != 0:
            raise Exception("Expect no notifications but received some")
        return True

    def getNotif(self, s):
        """Retrieves notifications"""
        try:
            data = s.recv(8192, socket.MSG_DONTWAIT)
            if "queue=TEST" not in data:
                raise Exception("Unexpected notification in socket")
            return 1
        except Exception, ex:
            if "Unexpected notification in socket" in str(ex):
                raise
        return 0
