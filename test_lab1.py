import pathlib
import re
import subprocess
import unittest

class TestLab1(unittest.TestCase):

    def _make():
        result = subprocess.run(['make'], capture_output=True, text=True)
        return result

    def _make_clean():
        result = subprocess.run(['make', 'clean'],
                                capture_output=True, text=True)
        return result

    @classmethod
    def setUpClass(cls):
        cls.make = cls._make().returncode == 0

    @classmethod
    def tearDownClass(cls):
        cls._make_clean()

    def test_3_commands(self):
        self.assertTrue(self.make, msg='make failed')
        cl_result = subprocess.run(('ls | cat | wc'),
                                capture_output=True, shell=True)
        pipe_result = subprocess.check_output(('./pipe', 'ls', 'cat', 'wc'))
        self.assertEqual(cl_result.stdout, pipe_result,
            msg=f"The output from ./pipe should be {cl_result.stdout} but got {pipe_result} instead.")
        self.assertTrue(self._make_clean, msg='make clean failed')
    
    def test_no_orphans(self):
        self.assertTrue(self.make, msg='make failed')
        subprocess.call(('strace', '-o', 'trace.log','./pipe','ls','wc','cat','cat'))
        ps = subprocess.Popen(['grep','-o','clone(','trace.log'], stdout=subprocess.PIPE)
        out1 = subprocess.check_output(('wc','-l'), stdin=ps.stdout)
        ps.wait()        
        ps.stdout.close()
        ps = subprocess.Popen(['grep','-o','wait','trace.log'], stdout=subprocess.PIPE)
        out2 = subprocess.check_output(('wc','-l'), stdin=ps.stdout)
        ps.wait()  
        ps.stdout.close()
        out1 = int(out1.decode("utf-8")[0])
        out2 = int(out2.decode("utf-8")[0])
        if out1 == out2 or out1 < out2:
            orphan_check = True
        else:
            orphan_check = False
        self.assertTrue(orphan_check, msg="Found orphan processes")
        subprocess.call(['rm', 'trace.log'])
        self.assertTrue(self._make_clean, msg='make clean failed')
    
    def test_bogus(self):
        self.assertTrue(self.make, msg='make failed')
        pipe_result = subprocess.run(('./pipe', 'ls', 'bogus'), stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
        self.assertTrue(pipe_result.returncode, msg='Bogus argument should cause an error, expect nonzero return code.')
        self.assertNotEqual(pipe_result.stderr, '', msg='Error should be reported to standard error.')
        self.assertTrue(self._make_clean, msg='make clean failed')

# import pathlib
# import re
# import subprocess
# import unittest

# class TestLab1(unittest.TestCase):

#     def _make():
#         result = subprocess.run(['make'], capture_output=True, text=True)
#         return result

#     def _make_clean():
#         result = subprocess.run(['make', 'clean'],
#                                 capture_output=True, text=True)
#         return result

#     @classmethod
#     def setUpClass(cls):
#         cls.make = cls._make().returncode == 0

#     @classmethod
#     def tearDownClass(cls):
#         cls._make_clean()

#     def test_3_commands(self):
#         self.assertTrue(self.make, msg='make failed')
#         cl_result = subprocess.run(('ls | cat | wc'),
#                                 capture_output=True, shell=True)
#         pipe_result = subprocess.check_output(('./pipe', 'ls', 'cat', 'wc'))
#         self.assertEqual(cl_result.stdout, pipe_result,
#             msg=f"The output from ./pipe should be {cl_result.stdout} but got {pipe_result} instead.")
#         self.assertTrue(self._make_clean, msg='make clean failed')
    
#     def test_no_orphans(self):
#         self.assertTrue(self.make, msg='make failed')
#         subprocess.call(('strace', '-o', 'trace.log','./pipe','ls','wc','cat','cat'))
#         ps = subprocess.Popen(['grep','-o','clone(','trace.log'], stdout=subprocess.PIPE)
#         out1 = subprocess.check_output(('wc','-l'), stdin=ps.stdout)
#         ps.wait()        
#         ps.stdout.close()
#         ps = subprocess.Popen(['grep','-o','wait','trace.log'], stdout=subprocess.PIPE)
#         out2 = subprocess.check_output(('wc','-l'), stdin=ps.stdout)
#         ps.wait()  
#         ps.stdout.close()
#         out1 = int(out1.decode("utf-8")[0])
#         out2 = int(out2.decode("utf-8")[0])
#         if out1 == out2 or out1 < out2:
#             orphan_check = True
#         else:
#             orphan_check = False
#         self.assertTrue(orphan_check, msg="Found orphan processes")
#         subprocess.call(['rm', 'trace.log'])
#         self.assertTrue(self._make_clean, msg='make clean failed')
    
#     def test_bogus(self):
#         self.assertTrue(self.make, msg='make failed')
#         pipe_result = subprocess.run(('./pipe', 'ls', 'bogus'), stdout=subprocess.PIPE,
#             stderr=subprocess.PIPE)
#         self.assertTrue(pipe_result.returncode, msg='Bogus argument should cause an error, expect nonzero return code.')
#         self.assertNotEqual(pipe_result.stderr, '', msg='Error should be reported to standard error.')
#         self.assertTrue(self._make_clean, msg='make clean failed')

#     # --- Additional tests below ---

#     def test_single_command(self):
#         """Test a single command without any pipes."""
#         self.assertTrue(self.make, msg='make failed')
#         cl_result = subprocess.run('ls', capture_output=True, shell=True)
#         pipe_result = subprocess.check_output(('./pipe', 'ls'))
#         self.assertEqual(cl_result.stdout, pipe_result,
#             msg=f"Single command output mismatch: expected {cl_result.stdout} but got {pipe_result}")
#         self.assertTrue(self._make_clean, msg='make clean failed')

#     def test_command_with_multiple_arguments(self):
#         """Test a command that contains multiple arguments in one command string."""
#         self.assertTrue(self.make, msg='make failed')
#         # Using echo to test multiple arguments
#         cl_result = subprocess.run("echo hello world", capture_output=True, shell=True)
#         # Note: We pass the entire command as one argument.
#         pipe_result = subprocess.check_output(('./pipe', 'echo hello world'))
#         self.assertEqual(cl_result.stdout, pipe_result,
#             msg=f"Command with multiple arguments output mismatch: expected {cl_result.stdout} but got {pipe_result}")
#         self.assertTrue(self._make_clean, msg='make clean failed')

#     def test_exit_status_propagation(self):
#         """Test that a pipeline with a failing last command propagates the nonzero exit code."""
#         self.assertTrue(self.make, msg='make failed')
#         # In this pipeline, the last command 'bogus' should fail, causing a nonzero exit status.
#         pipe_result = subprocess.run(('./pipe', 'echo hello', 'bogus'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#         self.assertNotEqual(pipe_result.returncode, 0,
#             msg="Nonzero exit code expected when last command fails.")
#         self.assertNotEqual(pipe_result.stderr, b'',
#             msg="An error message should be reported to standard error when a command fails.")
#         self.assertTrue(self._make_clean, msg='make clean failed')

