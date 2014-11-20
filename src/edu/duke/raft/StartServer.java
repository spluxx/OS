package edu.duke.raft;

import java.net.MalformedURLException;
import java.rmi.Naming;
import java.rmi.RemoteException;

public class StartServer {
  public static void main (String[] args) {
    if (args.length != 3) {
      System.out.println ("usage: java edu.duke.raft.StartServer -Djava.rmi.server.codebase=<codebase url> <int: rmiregistry port> <int: server id> <log file> <config file>");
      System.exit(1);
    }
    int port = Integer.parseInt (args[0]);    
    int id = Integer.parseInt (args[1]);
    String logPath = args[2];
    String configPath = args[3];
    
    String url = "rmi://localhost:" + port + "/S" + id;
    System.out.println ("Starting S" + id);
    System.out.println ("Binding server on rmiregistry " + url);

    int commitIndex = 0;

    RaftConfig config = new RaftConfig (configPath);
    RaftLog log = new RaftLog (logPath);
    int lastApplied = log.getLastIndex ();

    try {
      RaftState.initializeServer (config,
				  log,
				  lastApplied, 
				  port);
      RaftServerImpl server = new RaftServerImpl (id);
      server.setMode (new FollowerMode ());
      
      Naming.rebind(url, server);
    } catch (MalformedURLException me) {
      System.out.println ("S" + id + me.getMessage());
    } catch (RemoteException re) {
      System.out.println ("S" + id + re.getMessage());
    } catch (Exception e) {
      System.out.println ("S" + id + e.getMessage());
    }
  }  
}

  
