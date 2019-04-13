import ryu_ofctl

# part 1.3 tapping traffic
# h2 tapping traffic between h1 and h3

print "Tapping traffic for h1 <==> h3"

# get flow, set parameter
flow1 = ryu_ofctl.FlowEntry()
flow2 = ryu_ofctl.FlowEntry()
dpid = 1

# add action
act1 = ryu_ofctl.OutputAction(1)
act2 = ryu_ofctl.OutputAction(2)
act3 = ryu_ofctl.OutputAction(3)

# flow 1, from h1 to h2 & h3
flow1.in_port = 1
flow1.addAction(act2)
flow1.addAction(act3)
ryu_ofctl.insertFlow(dpid, flow1)

# flow 2, from h3 to h1 & h2
flow1.in_port = 3
flow1.addAction(act1)
flow1.addAction(act2)
ryu_ofctl.insertFlow(dpid, flow2)

print "Done tapping all traffic h1 <==> h3, with host h2 monitoring"