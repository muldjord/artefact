EXTRA_DIST = \
	artefact.conf.example \
	artefact.psql \
	artefact.in \
	oftdevproxy.in

sysconfdir=${prefix}/etc/artefact
dist_sysconf_DATA =

if WITH_ARTEFACT
dist_sysconf_DATA += \
	artefact.conf.example \
	artefact.psql

initddir=${prefix}/etc/init.d
dist_initd_DATA = \
	artefact
endif

xinetddir=${prefix}/etc/xinetd.d
dist_xinetd_DATA = 
if WITH_OFTDEVPROXY
dist_xinetd_DATA += \
	oftdevproxy
endif
