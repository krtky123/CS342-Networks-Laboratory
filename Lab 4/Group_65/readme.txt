Group 65
180123040 - Samiksha Sachdeva
180101033 - Kartikay Goel

Step 1: Install ns3 from nsnam.org and extract into a folder.
Run all the basic installation commands as specified in the documentation of ns3 to install various dependencies.

References: https://www.nsnam.org/docs/tutorial/html/getting-started.html#testing-ns-3

Step 2: Copy the two files((group65_main.cc and header.h) present in the "group65_sourcecode" directory of the zip submission to the scratch folder of ns3.

Running the code:
Run the following command from just outside the scratch folder:

./waf --run scratch/group65_main.cc

It will generate .plt files for all the graphs.
To convert the plt files to png images, run the following command:
gnuplot *.plt

Now, we have the following 6 plots:
1. Fairness vs Buffer Size
2. TCP throughput vs Buffer Size 
3. UDP Throughput vs Buffer Size 
4. TCP Throughput vs UDP Rate 
5. UDP Throughput vs UDP Rate
6. Remaining UDP Throughput vs UDP Rate

We have attached the report in the zip file.
