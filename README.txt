Configuration Interchange and Management
========================================

Getting started:

1. Get two D8 sites. We'll call them laurel and hardy.

2. Enable the module on both.

3. On hardy, go to Development > CIM > Upstream > Setup, and enter a local name
   and the url of laurel. You'll get redirected to laurel to authenticat (if 
   you're already logged in, all you have to do is provide a name for laurel
   to know hardy by).

4. Change some setting on laurel (cache setting or log messages to keep is
   two settings that's been converted to the new config system). Create a new
   configuration snapshot on the main page.

5. On hardy, go to Development > CIM > Upstream > Pull to pull the changes to
   hardy.

6. On hardy, change another thing (or the same), and create a new snapshot.
   Click on the ID and use Push to upstream to push it to laurel.
