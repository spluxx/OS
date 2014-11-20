package edu.duke.raft;

import java.net.MalformedURLException;
import java.rmi.Naming;
import java.rmi.RemoteException;

public class StartServer {
  public static void main (String[] args) {
    if (args.length != 2) {
      System.out.println ("usage: java edu.duke.raft.StartServer -Djava.rmi.server.codebase=<codebase url> <int: rmiregistry port> <int: server id>");
      System.exit(1);
    }
    int port = Integer.parseInt (args[0]);    
    int id = Integer.parseInt (args[1]);
    
    String url = "rmi://localhost:" + port + "/S" + id;
    System.out.println ("Starting S" + id);
    System.out.println ("Binding server on rmiregistry " + url);

    try {
      RaftState.initializeServer (0, -1, null, -1, -1, port);
      RaftServerImpl server = new RaftServerImpl (id);
      server.setState (new FollowerState ());
      
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

  
