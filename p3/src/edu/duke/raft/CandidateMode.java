package edu.duke.raft;
import java.util.Timer;
import java.util.concurrent.ThreadLocalRandom;

public class CandidateMode extends RaftMode {
  private boolean over;
  private Timer electionTimer, pollingTimer;
  private static int POLLING_INTERVAL = 70;
  private static int POLLING_TIMER_C = 10007;

  public void go () {
    synchronized (mLock) {
      doYourStuff();
      resetTimer();
      System.out.println ("("+this.getClass().getSimpleName()+")S" + mID + "." + mConfig.getCurrentTerm() + ": switched to candidate mode.");
    }
  }

  private void doYourStuff() {
    int t = mConfig.getCurrentTerm();
    over = false;
    mConfig.setCurrentTerm(t+1, mID);
    RaftResponses.setTerm(t+1);
    for(int i = 1 ; i <= mConfig.getNumServers() ; i ++) {
      if(mID == i) continue;
      remoteRequestVote(i, t+1, mID, mLog.getLastIndex(), mLog.getLastTerm());
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
      if(term < candidateTerm) { // to follower
	pollingTimer.cancel();
	electionTimer.cancel();
	RaftServerImpl.setMode(new FollowerMode());
      } return term; // I voted for myself anyways so ...
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
    synchronized (mLock) { // might have to handle the case when it does have to append entries
      int term = mConfig.getCurrentTerm ();
      if(term < leaderTerm) {
	pollingTimer.cancel();
	electionTimer.cancel();
	RaftServerImpl.setMode(new FollowerMode());
      } return term; // 
    }
  }

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    System.out.println (System.currentTimeMillis()%10000+"("+this.getClass().getSimpleName()+")S" + mID + "." + mConfig.getCurrentTerm() + ": TIMER " + timerID);

    synchronized (mLock) {
      if(mCommitIndex > mLastApplied) mCommitIndex = mLastApplied;
      if(timerID == POLLING_TIMER_C+mID) { // poll vote status
	int[] votes = RaftResponses.getVotes(mConfig.getCurrentTerm());
	if(votes == null) return; // term obsolete

	int nYes = 1;
	for(int i = 1 ; i < votes.length ; i ++) if(votes[i] == 0) nYes ++;
	if(nYes > mConfig.getNumServers()/2) {
	  over = true;
	  pollingTimer.cancel();
	  electionTimer.cancel();
	  System.out.println ("("+this.getClass().getSimpleName()+")S" + mID + "." + mConfig.getCurrentTerm() + ": qualified for leader mode.");
	  RaftServerImpl.setMode(new LeaderMode());
	}
	resetPolling();
      } else go(); // election finished without winner
    }
  }

  private void resetTimer() {
    if(pollingTimer != null) pollingTimer.cancel();
    if(electionTimer != null) electionTimer.cancel();
    int period = 0;
    if(mConfig.getTimeoutOverride() < 0)
      period = ThreadLocalRandom.current().nextInt(
          RaftMode.ELECTION_TIMEOUT_MIN, RaftMode.ELECTION_TIMEOUT_MAX);
    else period = mConfig.getTimeoutOverride();
    pollingTimer = super.scheduleTimer(POLLING_INTERVAL, POLLING_TIMER_C+mID);
    electionTimer = super.scheduleTimer(period, mID);
  }
   
  private void resetPolling() {
    if(pollingTimer != null) pollingTimer.cancel();
    if(!over) pollingTimer = super.scheduleTimer(POLLING_INTERVAL, POLLING_TIMER_C+mID);
  }
}
