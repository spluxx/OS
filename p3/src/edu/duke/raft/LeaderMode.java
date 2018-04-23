package edu.duke.raft;

import java.util.Timer;

public class LeaderMode extends RaftMode {
  private int nextIndex[];
  private int matchIndex[];
  private Timer heartbeatTimer, replyTimer, rpcTimer;
  private int REPLY_INTERVAL = 50;
  private int REPLY_C = 100000007;
  private int RPC_INTERVAL = 200;
  private int RPC_C = 12573748;

  public void go () {
    synchronized (mLock) {
      System.out.println ("S" + mID + "." + mConfig.getCurrentTerm() + ": switched to leader mode.");
      nextIndex = new int[mConfig.getNumServers()+1];
      matchIndex = new int[mConfig.getNumServers()+1];
      for(int i = 0 ; i < nextIndex.length ; i ++) nextIndex[i] = mLog.getLastIndex()+1;
      for(int i = 1 ; i <= mConfig.getNumServers() ; i ++) remoteAppendEntries(i, true);
      setTimer();
    }
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
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm ();
      if(term < candidateTerm) {
	mConfig.setCurrentTerm(candidateTerm, candidateID);
	heartbeatTimer.cancel(); replyTimer.cancel();
	RaftServerImpl.setMode(new FollowerMode());
	return 0;
      } else if(term > candidateTerm) {
	mConfig.setCurrentTerm(term, 0);
	return term; 
      } else {
  	if(mConfig.getVotedFor() == 0) {
	  // also additional shit
	  mConfig.setCurrentTerm(candidateTerm, candidateID);
	  return 0;
	} else {
	  mConfig.setCurrentTerm(candidateTerm, 0);
	  return term;
	}
      }
    }
  }
  

  // @param leader’s term
  // @param current leader
  // @param index of log entry before entries to append
  // @param term of log entry before entries to append
  // @param entries to append (in order of 0 to append.length-1)
  // @param index of highest committed entry
  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm ();
      int result = term;
      return result;
    }
  }

  private void remoteAppendEntries(int idx, boolean hb) {
    if(idx == mID) return;

    int term = mConfig.getCurrentTerm();
    int prevLogIndex = nextIndex[idx]-1;
    int prevLogTerm = prevLogIndex < 0 ? 0 : mLog.getEntry(nextIndex[idx]-1).term;
    Entry[] entries = null;

    if(!hb) {
      entries = new Entry[mLog.getLastIndex()-prevLogIndex];
      for(int i = prevLogIndex+1 ; i <= mLog.getLastIndex() ; i ++) {
	entries[i-prevLogIndex-1] = mLog.getEntry(i);
      }
    }
    remoteAppendEntries(idx, term, mID, prevLogIndex, prevLogTerm, entries, mCommitIndex);
  }

  private void setTimer() {
    heartbeatTimer = super.scheduleTimer(150, mID);
    rpcTimer = super.scheduleTimer(RPC_INTERVAL, RPC_C+mID);
    replyTimer = super.scheduleTimer(REPLY_INTERVAL, REPLY_C+mID);
  }

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized (mLock) {
      if(mCommitIndex > mLastApplied) mLastApplied = mCommitIndex;
      int nServers = mConfig.getNumServers();
      if(timerID == RPC_C+mID) { // batch RPC timer
	System.out.println("APPEND RPC!!!");
	for(int i = 1 ; i <= mConfig.getNumServers() ; i ++) remoteAppendEntries(i, false);
      } else if(timerID == REPLY_C+mID) { // reply timer
	int[] responses = RaftResponses.getAppendResponses(mConfig.getCurrentTerm());
	if(responses == null)  return;
	for(int i = 1 ; i <= nServers ; i ++) {
	  if(responses[i] < 0) continue;
	  else if(responses[i] == 0) { // YES!
	    matchIndex[i] = mLog.getLastIndex(); 
	    nextIndex[i] = mLog.getLastIndex() + 1;
	  } else { // NO!
	    if(responses[i] > mConfig.getCurrentTerm()) { // STEP DOWN!
	      mConfig.setCurrentTerm(responses[i], 0); 
	      heartbeatTimer.cancel(); replyTimer.cancel();
	      RaftServerImpl.setMode(new FollowerMode());
	    } else {
	      remoteAppendEntries(i, false);
	      nextIndex[i] --;
	    }
	  }
	}
	for(int i = mCommitIndex+1 ; i <= mLog.getLastIndex() ; i ++) {
	  int cnt = 0;
	  for(int j = 1 ; j <= mConfig.getNumServers() ; j ++) {
	    if(i <= matchIndex[j]) cnt ++;
	  } 
	  if(cnt > mConfig.getNumServers()/2 && 
	      mLog.getEntry(i).term == mConfig.getCurrentTerm()) mCommitIndex = i;
	}
	RaftResponses.clearAppendResponses(mConfig.getCurrentTerm());
      } else { // heartbeat
	for(int i = 1 ; i <= nServers ; i ++) remoteAppendEntries(i, true);
      }
    }
  }
}
