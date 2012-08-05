Configuration Interchange and Management
========================================

Getting started:

1. Get two D8 sites. We'll call them laurel and hardy.

2. Enable the module on both.

3. On hardy, go to Development > CIM > Upstream > Setup, and enter a
   local name and the url of laurel. You'll get redirected to laurel
   to authenticat (if you're already logged in, all you have to do is
   provide a name for laurel to know hardy by).

4. Change some setting on laurel (cache setting or log messages to
   keep is two settings that's been converted to the new config
   system). Create a new configuration snapshot on the main page.

5. On hardy, go to Development > CIM > Upstream > Pull to pull the
   changes to hardy.

6. On hardy, change another thing (or the same), and create a new
   snapshot.  Click on the ID and use Push to upstream to push it to
   laurel.

Inner workings:

CIMs way of dealing with configuration is heavily inspired by how git
manages revision history. 

Each snapshot consists of a reference to the snapshot before it, a log
entry, a set of changes, and possibly a full configuration dump (as to
not have to loop through the full history all the time).

Snapshots, changesets and dumps are identified by their sha, which
means that their id is bound to their content.

As CIM contains a simple diffing algorithm for producing changesets,
it's strait-forward to produce a new changset from the differences
between two arbitrary configuration dumps, whether they're part of the
same history or not.

A changeset is much like a git diff in that it contains both the old
and new values of each item, making it possible to check if the
changeset is appliable to another dump by checking if each item in the
new base is either the same old value, or the same as the new.

This architecture makes it possible to manage moving configuration
between sites, and allows for advanced management a la git merge and
rebase.
