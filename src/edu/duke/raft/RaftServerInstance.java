package edu.duke.raft;

import java.rmi.Remote;
import java.rmi.RemoteException;

import java.util.Timer;
import java.util.TimerTask;


public class RaftServer {

  private int mCurrentTerm;
  private int mElectionTimeout;
  private int mID;
  private RaftLog mLog;
  private int mVotedFor;

  public static void main () {
  }

  public RaftServer (int electionTimeout, int ID, RaftLog log) {
    mElectionTimeout = electionTimeout;
    mID = ID;
    mLog = log;
  }  

  public void fail () {
    System.exit (1);
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
			  int lastLogTerm) {
    return 0;    
  }  

  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm
			    Entry[] entries,
			    int leaderCommit) {
    return 0;    
  }

  private class RaftTimerTask extends TimerTask {
    // this lock ensure mutual exclusion between the thread that
    // handles network messages and the thread that handle the
    // election timeout

    private Object mLock;
    
    public RaftTimerTask (Object lock) {
      mLock = lock;
    }
    
    @Override
    public void run () {
    }
  }
  
}
