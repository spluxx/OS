package edu.duke.raft;

import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

public class RaftServerImpl extends UnicastRemoteObject 
  implements RaftServer {

  private int mID;
  private RaftState mState;
  
  // @param server's unique id
  public RaftServerImpl (int serverID) throws RemoteException {
    mID = serverID;
  }

  // @param the server's current state
  public void setState (RaftState state) {
    mState = state;
    state.go ();    
  }

  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0, if server votes for candidate; otherwise, server's
  // current term
  public int requestVote (int candidateTerm,
			  int candidateID,
			  int lastLogIndex,
			  int lastLogTerm) 
    throws RemoteException {
    System.out.println ("[S" + 
			mID + 
			"] Received requestVote request.");
    return 0;
  }

  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) 
    throws RemoteException  {
    System.out.println ("[S" +
			mID +
			"] Received appendEntries request.");
    return 0;
  }
}

  
