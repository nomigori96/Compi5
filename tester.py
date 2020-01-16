import os
import subprocess
import sys

if __name__ == "__main__":
    print("Compile With Makefile")
    subprocess.call("make", shell=True)
    print("Finished")
    print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   Compi5 tester   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
    cwd = os.getcwd()
    path = os.path.join(os.getcwd(), "tests")
    tests = [f for f in os.listdir(path) if (os.path.isfile(os.path.join(path, f)) and f.endswith(".in"))]
    print("")
    for test in tests:
        test = os.path.splitext(test)[0]
        print("\t" + str(test))
        subprocess.call("./hw5 < " + os.path.join(path, str(test) + ".in") + " >& " + os.path.join(path, str(test) + ".ll"), shell=True)
		subprocess.call("lli < " + os.path.join(path, str(test) + ".ll") + " > " + os.path.join(path, str(test) + ".res"), shell=True)
        subprocess.call("diff " + os.path.join(path, str(test) + ".out") + " " + os.path.join(path, str(test) + ".res"), shell=True)
    print("")