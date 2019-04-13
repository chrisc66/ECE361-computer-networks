#!/usr/bin/python

import sys
import re # For regex
import ryu_ofctl
from ryu_ofctl import *
from sets import Set



def main(macHostA, macHostB):
    print "Installing flows for %s <==> %s" % (macHostA, macHostB)

    ##### FEEL FREE TO MODIFY ANYTHING HERE #####
    try:
        pathA2B = dijkstras(macHostA, macHostB)
        installPathFlows(macHostA, macHostB, pathA2B)
    except:
        raise

    return 0



# Installs end-to-end bi-directional flows in all switches
def installPathFlows(macHostA, macHostB, pathA2B):
    
	for v in pathA2B[1:-1]: #intermediary nodes
		flow = ryu_ofctl.FlowEntry()
		act = ryu_ofctl.OutputAction(v['out_port'])
		
		flow.in_port = v['in_port']
		flow.dl_destination = re.sub(r'(?<=..)(..)', r':\1', hex(pathA2B[-1]['dpid'])[2:].zfill(12))
		flow.addAction(act)
		
		dpid = hex(pathA2B[0]['dpid'])
		ryu_ofctl.insertFlow(dpid, flow)
		
	return



# Returns List of neighbouring DPIDs
def findNeighbours(dpid):
    if type(dpid) not in (int, long) or dpid < 0:
        raise TypeError("DPID should be a positive integer value")

    neighbours = []

    links = ryu_ofctl.listSwitchLinks(hex(dpid))['links']
        
    for link in links:
		dpid1 = int(link['endpoint1']['dpid'], 16)
		dpid2 = int(link['endpoint2']['dpid'], 16)
		neighbour = dpid1 if dpid1 != dpid else dpid2
		if not neighbour in neighbours : neighbours.append(neighbour) 

    return neighbours



# Returns link connecting two DPIDs
def findLink(dpida, dpidb):
	if (type(dpida) not in (int, long) or dpida < 0 or type(dpidb) not in (int, long) or dpidb < 0):
		raise TypeError("DPID should be a positive integer value")
        
	links = ryu_ofctl.listSwitchLinks(hex(dpida))['links']
	
	for link in links:
		if (int(link['endpoint1']['dpid'], 16), int(link['endpoint2']['dpid'], 16)) == (dpida, dpidb):
			return [{'dpid': dpida, 'in_port': None, 'out_port': link['endpoint1']['port']},
					{'dpid': dpidb, 'in_port': link['endpoint2']['port'], 'out_port': None}]
	
	return None
	


# Calculates least distance path between A and B
# Returns detailed path (switch ID, input port, output port)
#   - Suggested data format is a List of Dictionaries
#       e.g.    [   {'dpid': 3, 'in_port': 1, 'out_port': 3},
#                   {'dpid': 2, 'in_port': 1, 'out_port': 2},
#                   {'dpid': 4, 'in_port': 3, 'out_port': 1},
#               ]
# Raises exception if either ingress or egress ports for the MACs can't be found
def dijkstras(macHostA, macHostB):

    # Optional helper function if you use suggested return format
	def nodeDict(dpid, in_port, out_port):
		assert type(dpid) in (int, long)
		assert type(in_port) is int
		assert type(out_port) is int
		return {'dpid': dpid, 'in_port': in_port, 'out_port': out_port}

    # Optional variables and data structures
    
	##### YOUR CODE HERE #####
	INFINITY = float('inf')
	src = int(macHostA, 16)
	dest = int(macHostB, 16)
	vertices = set([int(dpid, 16) for dpid in ryu_ofctl.listSwitches()['dpids']])
	dist = {v: INFINITY for v in vertices}
	prev = {v: None for v in vertices}
	dist[src] = 0
	
	while vertices:
		current = min(vertices, key = lambda v: dist[v])
		if dist[current] == INFINITY: break
		for neighbour in findNeighbours(current):
			w = 1 if findLink(current, neighbour) else INFINITY
			alt = dist[current] + w #length between neighbouring switch is 1
			if alt < dist[neighbour]:
				dist[neighbour] = alt
				prev[neighbour] = current
		vertices.remove(current)
			
	#reconsurt path
	if not prev[dest]: return [] #dest not reachable from src
	path = findLink(prev[dest], dest)
	dest = prev[dest]
	
	while dest in prev:
		if dest == src: break
		link = findLink(prev[dest], dest)
		path[0]['in_port'] = link[1]['in_port']
		path.insert(0, link[0])
		dest = prev[dest]

    # Some debugging output
    #print "leastDistNeighbour = %s" % leastDistNeighbour
    #print "distanceFromA = %s" % distanceFromA
    #print "pathAtoB = %s" % pathAtoB

	return path



# Validates the MAC address format and returns a lowercase version of it
def validateMAC(mac):
    invalidMAC = re.findall('[^0-9a-f:]', mac.lower()) or len(mac) != 17
    if invalidMAC:
        raise ValueError("MAC address %s has an invalid format" % mac)

    return mac.lower()

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print "This script installs bi-directional flows between two hosts"
        print "Expected usage: install_path.py <hostA's MAC> <hostB's MAC>"
    else:
        macHostA = validateMAC(sys.argv[1])
        macHostB = validateMAC(sys.argv[2])

        sys.exit( main(macHostA, macHostB) )
