
# Examples of candidates

An example with two investments candidates, one in semi-base generation
and one in network capacity, is given below.

![https://github.com/rte-antares-rpackage/antaresXpansion/raw/master/vignettes/example2nodes.png](../assets/media/image12.png)

The invested semi-base generation in *area 1* is shifted in the "virtual
node" *invest\_semibase*. Within the optimization process, the capacity
of the link between area 1 and *invest\_semibase* will be updated with
the new invested capacity.

The candidates.ini file for this example will be the following one. This
file has to be saved in the folder ./user/expansion/:

\[1\]

name = semibase

link = area1 - invest\_semibase

annual-cost-per-mw = 126000

unit-size = 200

max-units = 5

already\_installed\_capacity = 200

\[2\]

name = grid

link = area1 - area2

annual-cost-per-mw = 3000

unit-size = 500

max-units = 4

Another example with solar generation in a virtual node:

\[1\]

name = solar\_power

link = area1 - pv1

annual-cost-per-mw = 100000

max-investment = 10000

has-link-profile = True

link-profile = pv1.txt

Where pv1.txt is a text file, located in the ./user/expansion/capa/
folder of the study, and which contains the load factor time series of
the candidate (one column of 8760 values between 0 and 1, or two columns
if there is a direct and indirect profile on the link). When x MW of the
candidate *solar\_power* will be invested, the actual time series of
available power will be equal to the product of x and the time series
pv1.txt.
