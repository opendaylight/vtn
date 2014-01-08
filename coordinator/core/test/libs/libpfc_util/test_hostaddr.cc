/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * test_hostaddr.cc - Test for abstract host address.
 */
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <string>
#include <set>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <hostaddr_impl.h>
#include "test.h"
#include "random.hh"
#include "misc.hh"

#define TEST_HOST_NAME   "www.google.co.jp"

/*
 * Supported socket types.
 */
typedef struct {
    int  st_type;
    int  st_protocol;
} socktype_t;

typedef const socktype_t   csocktype_t;

static csocktype_t   socket_types[] = {
    { SOCK_STREAM, IPPROTO_TCP },
    { SOCK_DGRAM, IPPROTO_UDP },
#if     defined(SOCK_DCCP) && defined(IPPROTO_DCCP)
    { SOCK_DCCP, IPPROTO_DCCP },
#endif  /* defined(SOCK_DCCP) && defined(IPPROTO_DCCP) */
#ifdef  IPPROTO_UDPLITE
    { SOCK_DGRAM, IPPROTO_UDPLITE },
#endif  /* IPPROTO_UDPLITE */
#ifdef  IPPROTO_SCTP
    { SOCK_STREAM, IPPROTO_SCTP },
#ifdef  SOCK_SEQPACKET
    { SOCK_SEQPACKET, IPPROTO_SCTP },
#endif  /* SOCK_SEQPACKET */
#endif  /* IPPROTO_SCTP */
    { SOCK_RAW, 0 },
};

static csocktype_t *
get_socktype(int type, int proto)
{
    for (csocktype_t *stp(socket_types); stp < PFC_ARRAY_LIMIT(socket_types);
         stp++) {
        if (stp->st_type == type && stp->st_protocol == proto) {
            return stp;
        }
    }

    return NULL;
}

static const char	*addrtype_local[] = {
    "/LOCAL",
    "/local",
    "/Local",
    "/LocaL",
};

static const char	*addrtype_ipv4[] = {
    "/INET4",
    "/inet4",
    "/Inet4",
    "/iNet4",
};

static const char	*addrtype_ipv6[] = {
    "/INET6",
    "/inet6",
    "/Inet6",
    "/iNet6",
};

static const uint32_t	ipv6_scopes[] = {
    1,
    2,
    10,
    20,
    100,
    1000,
    UINT32_MAX,
};

/*
 * Network interface list.
 */
class NetworkInterfaces
{
public:
    NetworkInterfaces() : _interfaces(if_nameindex()) {}

    ~NetworkInterfaces()
    {
        if (_interfaces != NULL) {
            if_freenameindex(_interfaces);
        }
    }

    inline struct if_nameindex *
    get(void)
    {
        return _interfaces;
    }

private:
    struct if_nameindex  *_interfaces;
};

/*
 * List of struct addrinfo.
 */
class AddrInfoList
{
public:
    AddrInfoList(struct addrinfo *info) : _list(info) {}

    ~AddrInfoList()
    {
        freeaddrinfo(_list);
    }

    bool  contains(int family, void *addr);

private:
    struct addrinfo   *_list;
};

bool
AddrInfoList::contains(int family, void *addr)
{
    for (struct addrinfo *aip(_list); aip != NULL; aip = aip->ai_next) {
        struct sockaddr  *saddr(aip->ai_addr);

        if (saddr->sa_family != family) {
            continue;
        }

        if (family == AF_INET) {
            struct sockaddr_in *sin(reinterpret_cast<struct sockaddr_in *>
                                    (saddr));

            if (memcmp(addr, &sin->sin_addr, sizeof(sin->sin_addr)) == 0) {
                return true;
            }
        }
        else {
            struct sockaddr_in6 *sin6(reinterpret_cast<struct sockaddr_in6 *>
                                      (saddr));

            if (memcmp(addr, &sin6->sin6_addr, sizeof(sin6->sin6_addr)) == 0) {
                return true;
            }
        }
    }

    return false;
}

class SockAddr
{
public:
    SockAddr()
    {
        memset(&_saddr, 0, sizeof(_saddr));
    }

    ~SockAddr()
    {
        pfc_sockaddr_destroy(&_saddr);
    }

    inline int
    init(const pfc_hostaddr_t &haddr, const char *PFC_RESTRICT service,
         const struct addrinfo *PFC_RESTRICT hints)
    {
        if (_saddr.sa_addr != NULL) {
            pfc_sockaddr_destroy(&_saddr);
        }

        return pfc_sockaddr_init(&_saddr, &haddr, service, hints);
    }

    const pfc_sockaddr_t *
    get(void) const
    {
        return &_saddr;
    }

private:
    pfc_sockaddr_t  _saddr;
};

static void
random_inet4(RandomGenerator &rand, struct in_addr &iaddr, char *text,
             size_t textlen)
{
    RANDOM_INTEGER(rand, iaddr.s_addr);
    ASSERT_TRUE(inet_ntop(AF_INET, &iaddr, text, textlen) != NULL);
}

static void
random_inet6(RandomGenerator &rand, struct in6_addr &iaddr, char *text,
             size_t textlen)
{
    try {
        rand.fillRandom(iaddr.s6_addr, sizeof(iaddr.s6_addr));
    }
    catch (const RandomError &e) {
        int  err(e.getError());

        if (err == 0) {
            FAIL() << e.what();
        }
        else {
            FAIL() << e.what() << ": " << strerror(err);
        }
    }

    ASSERT_TRUE(inet_ntop(AF_INET6, &iaddr, text, textlen) != NULL);
}

/*
 * Local host address test.
 */
static void
test_local(pfc_hostaddr_t *addr)
{
    hostaddr_t  *hap(reinterpret_cast<hostaddr_t *>(addr));

    ASSERT_EQ(AF_UNIX, pfc_hostaddr_gettype(addr));

    uint8_t addrbuf[4] = {1, 2, 3, 4};
    size_t  size(sizeof(addrbuf));
    ASSERT_EQ(0, pfc_hostaddr_getaddr(addr, addrbuf, &size));
    ASSERT_EQ(0U, size);
    for (uint8_t i(0); i < PFC_ARRAY_CAPACITY(addrbuf); i++) {
        ASSERT_EQ(i + 1, addrbuf[i]);
    }

    ASSERT_EQ(0ULL, hap->ha_addr.inet6[0]);
    ASSERT_EQ(0ULL, hap->ha_addr.inet6[1]);

    // Scope ID must be zero.
    uint32_t  scope;
    ASSERT_EQ(0, pfc_hostaddr_getscope(addr, &scope));
    ASSERT_EQ(0U, scope);

    // tostring/fromstring test.
    pfc_hostaddr_t newaddr;
    char           buf[32];
    ASSERT_EQ(ENOSPC, pfc_hostaddr_tostring(addr, buf, 0, 0));
    ASSERT_EQ(0, pfc_hostaddr_tostring(addr, buf, 1, 0));
    ASSERT_STREQ("", buf);
    ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, buf));
    ASSERT_EQ(0, pfc_hostaddr_compare(addr, &newaddr));

    for (uint32_t sz(0); sz < 6; sz++) {
        ASSERT_EQ(ENOSPC, pfc_hostaddr_tostring(addr, buf, sz,
                                                PFC_HA2STR_TYPE));
    }

    for (uint32_t sz(6); sz <= sizeof(buf); sz++) {
        buf[0] = '\0';
        ASSERT_EQ(0, pfc_hostaddr_tostring(addr, buf, sz, PFC_HA2STR_TYPE));
        ASSERT_STREQ("LOCAL", buf);

        ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, buf));
        ASSERT_EQ(0, pfc_hostaddr_compare(addr, &newaddr));
    }
}

/*
 * IPv4 address test.
 */
static void
test_inet4(pfc_hostaddr_t *addr, struct in_addr *netaddr, const char *textaddr)
{
    ASSERT_EQ(AF_INET, pfc_hostaddr_gettype(addr));

    struct in_addr  iaddr;
    size_t size;
    for (size_t sz(0); sz < sizeof(iaddr); sz++) {
        size = sz;
        ASSERT_EQ(ENOSPC, pfc_hostaddr_getaddr(addr, &iaddr, &size));
    }

    for (size_t sz(0); sz <= 32; sz++) {
        size = sizeof(iaddr) + sz;
        ASSERT_EQ(0, pfc_hostaddr_getaddr(addr, &iaddr, &size));
        ASSERT_EQ(sizeof(iaddr), size);
        ASSERT_EQ(netaddr->s_addr, iaddr.s_addr);
    }

    {
        pfc_hostaddr_t  newaddr;

        ASSERT_EQ(EINVAL, pfc_hostaddr_init_inet4(NULL, &iaddr));
        ASSERT_EQ(0, pfc_hostaddr_init_inet4(&newaddr, &iaddr));
        ASSERT_EQ(0, pfc_hostaddr_compare(&newaddr, addr));
    }

    // Scope ID must be zero.
    uint32_t  scope;
    ASSERT_EQ(0, pfc_hostaddr_getscope(addr, &scope));
    ASSERT_EQ(0U, scope);

    // tostring/fromstring test.
    pfc_hostaddr_t newaddr;
    char           buf[64];
    uint32_t       tlen(strlen(textaddr));
    ASSERT_LT(tlen, PFC_HOSTADDR_STRSIZE_INET4);

    for (uint32_t sz(0); sz <= tlen; sz++) {
        ASSERT_EQ(ENOSPC, pfc_hostaddr_tostring(addr, buf, sz, 0));
    }

    for (uint32_t sz(1); sz <= 32; sz++) {
        buf[0] = '\0';
        size = tlen + sz;
        ASSERT_EQ(0, pfc_hostaddr_tostring(addr, buf, size, 0));
        ASSERT_STREQ(textaddr, buf);

        ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, buf));
        ASSERT_EQ(0, pfc_hostaddr_compare(addr, &newaddr));
    }

    std::string reqstr(textaddr);
    reqstr.append("/INET4");
    tlen = reqstr.length();
    ASSERT_LT(tlen, PFC_HOSTADDR_STRSIZE);

    for (uint32_t sz(0); sz <= tlen; sz++) {
        ASSERT_EQ(ENOSPC, pfc_hostaddr_tostring(addr, buf, sz,
                                                PFC_HA2STR_TYPE));
    }

    for (uint32_t sz(1); sz <= 32; sz++) {
        buf[0] = '\0';
        size = tlen + sz;
        ASSERT_EQ(0, pfc_hostaddr_tostring(addr, buf, size, PFC_HA2STR_TYPE));
        ASSERT_STREQ(reqstr.c_str(), buf);

        ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, buf));
        ASSERT_EQ(0, pfc_hostaddr_compare(addr, &newaddr));
    }
}

/*
 * IPv6 address test.
 */
static void
test_inet6(pfc_hostaddr_t *addr, struct in6_addr *netaddr,
           const char *textaddr, uint32_t reqscope = 0)
{
    ASSERT_EQ(AF_INET6, pfc_hostaddr_gettype(addr));

    struct in6_addr  iaddr;
    size_t size;
    for (size_t sz(0); sz < sizeof(iaddr); sz++) {
        size = sz;
        ASSERT_EQ(ENOSPC, pfc_hostaddr_getaddr(addr, &iaddr, &size));
    }

    for (size_t sz(0); sz <= 32; sz++) {
        size = sizeof(iaddr) + sz;
        ASSERT_EQ(0, pfc_hostaddr_getaddr(addr, &iaddr, &size));
        ASSERT_EQ(sizeof(iaddr), size);
        if (memcmp(netaddr->s6_addr, iaddr.s6_addr, sizeof(iaddr.s6_addr))) {
            char  buf[64];

            ASSERT_TRUE(inet_ntop(AF_INET6, &iaddr, buf, sizeof(buf)) != NULL);
            ASSERT_EQ(0, memcmp(netaddr->s6_addr, iaddr.s6_addr,
                                sizeof(iaddr.s6_addr)))
                << "textaddr=" << textaddr << ": iaddr=" << buf;
        }
    }

    if (reqscope == 0) {
        pfc_hostaddr_t  newaddr;

        ASSERT_EQ(EINVAL, pfc_hostaddr_init_inet6(NULL, &iaddr, 0));
        ASSERT_EQ(0, pfc_hostaddr_init_inet6(&newaddr, &iaddr, 0));
        ASSERT_EQ(0, pfc_hostaddr_compare(&newaddr, addr));

        for (uint32_t i = 1U; i <= 10U; i++) {
            ASSERT_EQ(0, pfc_hostaddr_init_inet6(&newaddr, &iaddr, i));
            uint32_t scope;
            ASSERT_EQ(0, pfc_hostaddr_getscope(&newaddr, &scope));
            ASSERT_EQ(i, scope);
            ASSERT_NE(0, pfc_hostaddr_compare(&newaddr, addr));
        }
    }

    uint32_t scope;
    ASSERT_EQ(0, pfc_hostaddr_getscope(addr, &scope));
    ASSERT_EQ(reqscope, scope);

    // tostring/fromstring test.
    pfc_hostaddr_t newaddr;
    char           buf[64];
    uint32_t       tlen(strlen(textaddr));
    ASSERT_LT(tlen, PFC_HOSTADDR_STRSIZE_INET6);

    for (uint32_t sz(0); sz <= tlen; sz++) {
        ASSERT_EQ(ENOSPC, pfc_hostaddr_tostring(addr, buf, sz, 0));
    }

    for (uint32_t sz(1); sz <= 32; sz++) {
        buf[0] = '\0';
        size = tlen + sz;
        ASSERT_EQ(0, pfc_hostaddr_tostring(addr, buf, size, 0));
        ASSERT_STREQ(textaddr, buf);

        ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, buf));
        ASSERT_EQ(0, pfc_hostaddr_compare(addr, &newaddr));
    }

    std::string reqstr(textaddr);
    reqstr.append("/INET6");
    tlen = reqstr.length();
    ASSERT_LT(tlen, PFC_HOSTADDR_STRSIZE);

    for (uint32_t sz(0); sz <= tlen; sz++) {
        ASSERT_EQ(ENOSPC, pfc_hostaddr_tostring(addr, buf, sz,
                                                PFC_HA2STR_TYPE));
    }

    for (uint32_t sz(1); sz <= 32; sz++) {
        buf[0] = '\0';
        size = tlen + sz;
        ASSERT_EQ(0, pfc_hostaddr_tostring(addr, buf, size, PFC_HA2STR_TYPE));
        ASSERT_STREQ(reqstr.c_str(), buf);

        ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, buf));
        ASSERT_EQ(0, pfc_hostaddr_compare(addr, &newaddr));
    }
}

/*
 * sockaddr_in test.
 */
static void
test_sockaddr_in(SockAddr &sa, struct in_addr &iaddr, uint32_t port, int type)
{
    const pfc_sockaddr_t  *sap(sa.get());
    ASSERT_TRUE(sap->sa_addr != NULL);

    const struct sockaddr_in  *saddr
        (reinterpret_cast<const struct sockaddr_in *>(sap->sa_addr));
    ASSERT_EQ(AF_INET, saddr->sin_family);
    ASSERT_EQ(port, ntohs(saddr->sin_port));
    ASSERT_EQ(0, memcmp(&iaddr, &saddr->sin_addr, sizeof(iaddr)));
    ASSERT_EQ(sizeof(*saddr), sap->sa_addrlen);

    csocktype_t *stp(get_socktype(sap->sa_socktype, sap->sa_protocol));
    ASSERT_TRUE(stp != NULL)
        << "type=" << sap->sa_socktype
        << " proto=" << sap->sa_protocol;

    if (type != 0) {
        ASSERT_EQ(type, sap->sa_socktype);
    }
}

/*
 * sockaddr_in6 test.
 */
static void
test_sockaddr_in6(SockAddr &sa, struct in6_addr &iaddr, uint32_t port,
                  int type, uint32_t scope = 0)
{
    const pfc_sockaddr_t  *sap(sa.get());
    ASSERT_TRUE(sap->sa_addr != NULL);

    const struct sockaddr_in6  *saddr
        (reinterpret_cast<const struct sockaddr_in6 *>(sap->sa_addr));
    ASSERT_EQ(AF_INET6, saddr->sin6_family);
    ASSERT_EQ(port, ntohs(saddr->sin6_port));
    ASSERT_EQ(0U, saddr->sin6_flowinfo);
    ASSERT_EQ(scope, saddr->sin6_scope_id);
    ASSERT_EQ(0, memcmp(&iaddr, &saddr->sin6_addr, sizeof(iaddr)));
    ASSERT_EQ(sizeof(*saddr), sap->sa_addrlen);

    csocktype_t *stp(get_socktype(sap->sa_socktype, sap->sa_protocol));
    ASSERT_TRUE(stp != NULL)
        << "type=" << sap->sa_socktype
        << " proto=" << sap->sa_protocol;

    if (type != 0) {
        ASSERT_EQ(type, sap->sa_socktype);
    }
}

/*
 * Below are test cases.
 */

/*
 * Test case for local host address.
 */
TEST(hostaddr, init_local)
{
    ASSERT_EQ(EINVAL, pfc_hostaddr_init_local(NULL));

    /* AF_UNIX tests. */
    pfc_hostaddr_t  addr;
    ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_UNIX, NULL));
    test_local(&addr);
    RETURN_ON_ERROR();

    ASSERT_EQ(0, pfc_hostaddr_init_local(&addr));
    test_local(&addr);

    // pfc_hostaddr_fromstring() treats NULL as local address.
    pfc_hostaddr_t  newaddr;
    ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, NULL));
    ASSERT_EQ(0, pfc_hostaddr_compare(&addr, &newaddr));

    // pfc_hostaddr_fromstring() constructs local address if the string ends
    // with "/LOCAL".
    const char *hosts[] = {
        "",
        "foo",
        "bar",
    };

    for (const char **h(hosts); h < PFC_ARRAY_LIMIT(hosts); h++) {
        for (const char **s(addrtype_local);
             s < PFC_ARRAY_LIMIT(addrtype_local); s++) {
            std::string  laddr(*h);
            laddr.append(*s);

            ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, laddr.c_str()));
            ASSERT_EQ(0, pfc_hostaddr_compare(&addr, &newaddr));
        }
    }
}

/*
 * Test case for IPv4 host address with numerical address.
 */
TEST(hostaddr, init_ipv4)
{
    /* Invalid IPv4 address. */
    pfc_hostaddr_t  addr;
    int  gerr(pfc_hostaddr_init(&addr, AF_INET, "100.200.300.400"));
    ASSERT_NE(0, gerr);

    gerr = pfc_hostaddr_fromstring(&addr, "100.200.300.400/INET4");
    ASSERT_NE(0, gerr);

    /* NULL address must be converted into loopback. */
    struct in_addr  iaddr;
    char  buf[PFC_HOSTADDR_STRSIZE_INET4];
    snprintf(buf, sizeof(buf), "127.0.0.1");
    ASSERT_EQ(1, inet_pton(AF_INET, buf, &iaddr));
    ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_INET, NULL));
    test_inet4(&addr, &iaddr, buf);
    RETURN_ON_ERROR();
    ASSERT_EQ(0, pfc_hostaddr_init_inet4(&addr, NULL));
    test_inet4(&addr, &iaddr, buf);

    for (const char **s(addrtype_ipv4); s < PFC_ARRAY_LIMIT(addrtype_ipv4);
         s++) {
        pfc_hostaddr_t  newaddr;

        ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, *s));
        ASSERT_EQ(0, pfc_hostaddr_compare(&addr, &newaddr));
    }

    RandomGenerator  rand;
    for (uint32_t loop(0); loop < 1000; loop++) {
        random_inet4(rand, iaddr, buf, sizeof(buf));
        RETURN_ON_ERROR();

        ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_INET, buf));
        test_inet4(&addr, &iaddr, buf);
        RETURN_ON_ERROR();
    }
}

/*
 * Test case for IPv4 host address with hostname.
 * Note that this test requires DNS.
 */
TEST(hostaddr, DISABLED_init_ipv4_host)
{
    struct addrinfo  hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_flags = AI_NUMERICSERV;
    struct addrinfo  *result;
    ASSERT_EQ(0, getaddrinfo(TEST_HOST_NAME, "23", &hints, &result));
    AddrInfoList  list(result);

    pfc_hostaddr_t  addr;
    struct in_addr  iaddr;
    size_t size(sizeof(iaddr));
    ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_INET, TEST_HOST_NAME));
    ASSERT_EQ(AF_INET, pfc_hostaddr_gettype(&addr));
    ASSERT_EQ(0, pfc_hostaddr_getaddr(&addr, &iaddr, &size));
    ASSERT_EQ(sizeof(iaddr), size);
    ASSERT_TRUE(list.contains(AF_INET, &iaddr));

    // fromstring test.
    for (const char **s(addrtype_ipv4); s < PFC_ARRAY_LIMIT(addrtype_ipv4);
         s++) {
        std::string  textaddr(TEST_HOST_NAME);
        textaddr.append(*s);

        size = sizeof(iaddr);
        ASSERT_EQ(0, pfc_hostaddr_fromstring(&addr, textaddr.c_str()));
        ASSERT_EQ(AF_INET, pfc_hostaddr_gettype(&addr));
        ASSERT_EQ(0, pfc_hostaddr_getaddr(&addr, &iaddr, &size));
        ASSERT_EQ(sizeof(iaddr), size);
        ASSERT_TRUE(list.contains(AF_INET, &iaddr));
    }
}

/*
 * Test case for IPv6 host address with numerical address.
 */
TEST(hostaddr, init_ipv6)
{
    /* Invalid IPv6 address. */
    pfc_hostaddr_t  addr;
    int  gerr(pfc_hostaddr_init(&addr, AF_INET6, "::ffff:gggg:hhhh:iiii"));
    ASSERT_NE(0, gerr);

    gerr = pfc_hostaddr_fromstring(&addr, "::ffff:gggg:hhhh:iiii/INET6");
    ASSERT_NE(0, gerr);

    /* NULL address must be converted into loopback. */
    struct in6_addr  iaddr;
    char  buf[PFC_HOSTADDR_STRSIZE_INET6];
    snprintf(buf, sizeof(buf), "::1");
    ASSERT_EQ(1, inet_pton(AF_INET6, buf, &iaddr));
    ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_INET6, NULL));
    test_inet6(&addr, &iaddr, buf);
    RETURN_ON_ERROR();
    ASSERT_EQ(0, pfc_hostaddr_init_inet6(&addr, NULL, 0));
    test_inet6(&addr, &iaddr, buf);

    for (const char **s(addrtype_ipv6); s < PFC_ARRAY_LIMIT(addrtype_ipv6);
         s++) {
        pfc_hostaddr_t  newaddr;

        ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, *s));
        ASSERT_EQ(0, pfc_hostaddr_compare(&addr, &newaddr));
    }

    RandomGenerator  rand;
    for (uint32_t loop(0); loop < 1000; loop++) {
        random_inet6(rand, iaddr, buf, sizeof(buf));
        RETURN_ON_ERROR();

        ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_INET6, buf));
        test_inet6(&addr, &iaddr, buf);
        RETURN_ON_ERROR();
    }
}

/*
 * Test case for IPv6 host address with hostname.
 * Note that this test requires DNS.
 */
TEST(hostaddr, DISABLED_init_ipv6_host)
{
    struct addrinfo  hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET6;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo  *result;
    ASSERT_EQ(0, getaddrinfo(TEST_HOST_NAME, "23", &hints, &result));
    AddrInfoList  list(result);

    pfc_hostaddr_t  addr;
    struct in6_addr  iaddr;
    size_t size(sizeof(iaddr));
    ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_INET6, TEST_HOST_NAME));
    ASSERT_EQ(AF_INET6, pfc_hostaddr_gettype(&addr));
    ASSERT_EQ(0, pfc_hostaddr_getaddr(&addr, &iaddr, &size));
    ASSERT_EQ(sizeof(iaddr), size);
    ASSERT_TRUE(list.contains(AF_INET6, &iaddr));

    // fromstring test.
    for (const char **s(addrtype_ipv6); s < PFC_ARRAY_LIMIT(addrtype_ipv6);
         s++) {
        std::string  textaddr(TEST_HOST_NAME);
        textaddr.append(*s);

        size = sizeof(iaddr);
        ASSERT_EQ(0, pfc_hostaddr_fromstring(&addr, textaddr.c_str()));
        ASSERT_EQ(AF_INET6, pfc_hostaddr_gettype(&addr));
        ASSERT_EQ(0, pfc_hostaddr_getaddr(&addr, &iaddr, &size));
        ASSERT_EQ(sizeof(iaddr), size);
        ASSERT_TRUE(list.contains(AF_INET6, &iaddr));
    }
}

/*
 * Test case for IPv6 address with scope ID.
 */
TEST(hostaddr, init_ipv6_scope)
{
    // Specify scope by index.
    for (const uint32_t *scp(ipv6_scopes); scp < PFC_ARRAY_LIMIT(ipv6_scopes);
         scp++) {
        uint32_t     scope(*scp);
        const char   *v6addr("aaaa:bbbb:cccc:dddd:eeee:ffff:1111:2222");
        std::string  textaddr(v6addr);

        struct in6_addr  iaddr;
        ASSERT_EQ(1, inet_pton(AF_INET6, v6addr, &iaddr));

        char  buf[32];
        snprintf(buf, sizeof(buf), "%u", scope);
        textaddr.append("%");
        textaddr.append(buf);
        pfc_hostaddr_t  haddr;
        ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET6, textaddr.c_str()));
        test_inet6(&haddr, &iaddr, textaddr.c_str(), scope);
    }

    // Specify scope by the name of network interface.
    NetworkInterfaces nif;
    struct if_nameindex *ifp(nif.get());
    if (ifp != NULL) {
        for (; ifp->if_index != 0; ifp++) {
            const char  *v6addr("fe80::219:eeff:1234:5678");
            std::string  textaddr(v6addr);

            struct in6_addr  iaddr;
            ASSERT_EQ(1, inet_pton(AF_INET6, v6addr, &iaddr));

            textaddr.append("%");
            textaddr.append(ifp->if_name);
            pfc_hostaddr_t  haddr1, haddr2, haddr3;
            ASSERT_EQ(0, pfc_hostaddr_init(&haddr1, AF_INET6,
                                           textaddr.c_str()));
            ASSERT_EQ(0, pfc_hostaddr_fromstring(&haddr2, textaddr.c_str()));
            textaddr.append("/INET6");
            ASSERT_EQ(0, pfc_hostaddr_fromstring(&haddr3, textaddr.c_str()));

            char  buf[32];
            snprintf(buf, sizeof(buf), "%u", ifp->if_index);
            textaddr.assign(v6addr);
            textaddr.append("%");
            textaddr.append(buf);
            test_inet6(&haddr1, &iaddr, textaddr.c_str(), ifp->if_index);
            test_inet6(&haddr2, &iaddr, textaddr.c_str(), ifp->if_index);
            test_inet6(&haddr3, &iaddr, textaddr.c_str(), ifp->if_index);
        }
    }
}

/*
 * Test case for unspecified address family.
 */
TEST(hostaddr, init_unspec)
{
    /* Invalid address. */
    const char      *name("unknown-host-name::");
    pfc_hostaddr_t  addr;
    int  gerr(pfc_hostaddr_init(&addr, AF_UNSPEC, name));
    ASSERT_NE(0, gerr);

    gerr = pfc_hostaddr_fromstring(&addr, name);
    ASSERT_NE(0, gerr);

    /* IPv4 address. */
    RandomGenerator  rand;
    for (uint32_t loop(0); loop < 500; loop++) {
        struct in_addr  iaddr;
        char  buf[PFC_HOSTADDR_STRSIZE_INET4];

        random_inet4(rand, iaddr, buf, sizeof(buf));
        RETURN_ON_ERROR();

        ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_UNSPEC, buf));
        test_inet4(&addr, &iaddr, buf);
        RETURN_ON_ERROR();
    }

    /* IPv6 address. */
    for (uint32_t loop(0); loop < 500; loop++) {
        struct in6_addr  iaddr;
        char  buf[PFC_HOSTADDR_STRSIZE_INET6];

        random_inet6(rand, iaddr, buf, sizeof(buf));
        RETURN_ON_ERROR();

        ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_UNSPEC, buf));
        test_inet6(&addr, &iaddr, buf);
        RETURN_ON_ERROR();
    }
}

/*
 * Test case for unspecified address family with hostname.
 * Note that this test requires DNS.
 */
TEST(hostaddr, DISABLED_init_unspec_host)
{
    struct addrinfo  hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo  *result;
    ASSERT_EQ(0, getaddrinfo(TEST_HOST_NAME, "23", &hints, &result));
    AddrInfoList  list(result);

    pfc_hostaddr_t  addr;
    struct in6_addr  iaddr;
    size_t size(sizeof(iaddr));
    ASSERT_EQ(0, pfc_hostaddr_init(&addr, AF_UNSPEC, TEST_HOST_NAME));

    int family(pfc_hostaddr_gettype(&addr));
    ASSERT_EQ(0, pfc_hostaddr_getaddr(&addr, &iaddr, &size));

    if (family == AF_INET) {
        ASSERT_EQ(sizeof(struct in_addr), size);
    }
    else {
        ASSERT_EQ(AF_INET6, family);
        ASSERT_EQ(sizeof(iaddr), size);
    }

    ASSERT_TRUE(list.contains(family, &iaddr));

    // fromstring test.
    pfc_hostaddr_t  newaddr;
    ASSERT_EQ(0, pfc_hostaddr_fromstring(&newaddr, TEST_HOST_NAME));

    size = sizeof(iaddr);
    ASSERT_EQ(0, pfc_hostaddr_getaddr(&addr, &iaddr, &size));

    family = pfc_hostaddr_gettype(&addr);
    if (family == AF_INET) {
        ASSERT_EQ(sizeof(struct in_addr), size);
    }
    else {
        ASSERT_EQ(AF_INET6, family);
        ASSERT_EQ(sizeof(iaddr), size);
    }

    ASSERT_TRUE(list.contains(family, &iaddr));
}

/*
 * Error test case for pfc_hostaddr_init().
 */
TEST(hostaddr, init_error)
{
    errno = 0;
    ASSERT_EQ(EAI_SYSTEM, pfc_hostaddr_init(NULL, 0, NULL));
    ASSERT_EQ(EINVAL, errno);

    /* Unsupported address family. */
    pfc_hostaddr_t  addr;

    for (int family(0); family <= 100; family++) {
        if (family == AF_UNSPEC || family == AF_UNIX || family == AF_INET ||
            family == AF_INET6) {
            continue;
        }

        ASSERT_EQ(EAI_FAMILY, pfc_hostaddr_init(&addr, family, "addr"));
    }
}

/*
 * Error test case for pfc_hostaddr_fromstring().
 */
TEST(hostaddr, fromstring_error)
{
    errno = 0;

    ASSERT_EQ(EAI_SYSTEM, pfc_hostaddr_fromstring(NULL, NULL));
    ASSERT_EQ(EINVAL, errno);

    // Invalid address type.
    pfc_hostaddr_t  addr;
    ASSERT_EQ(EAI_FAMILY, pfc_hostaddr_fromstring(&addr, "foo/"));
    ASSERT_EQ(EAI_FAMILY, pfc_hostaddr_fromstring(&addr, "foo/BAD_TYPE"));
}

/*
 * Error test case for pfc_hostaddr_tostring().
 */
TEST(hostaddr, tostring_error)
{
    pfc_hostaddr_t  addr;
    memset(&addr, 0, sizeof(addr));

    char  buf[PFC_HOSTADDR_STRSIZE];
    ASSERT_EQ(EINVAL, pfc_hostaddr_tostring(NULL, buf, sizeof(buf), 0));
    ASSERT_EQ(EINVAL, pfc_hostaddr_tostring(&addr, buf, sizeof(buf), 0));

    ASSERT_EQ(0, pfc_hostaddr_init_local(&addr));
    ASSERT_EQ(EINVAL, pfc_hostaddr_tostring(&addr, NULL, sizeof(buf), 0));

    // Unaligned attribute pointer test.
    hostaddr_t      broken;
    pfc_hostaddr_t  *haddr(reinterpret_cast<pfc_hostaddr_t *>(&broken));
    ASSERT_EQ(sizeof(broken), sizeof(addr));

    for (uintptr_t off(1); off < sizeof(void *); off++) {
        memcpy(&broken, &addr, sizeof(addr));

        uintptr_t  ptr(reinterpret_cast<uintptr_t>(broken.ha_attr));

        broken.ha_attr = reinterpret_cast<haddr_cattr_t *>(ptr + off);
        ASSERT_EQ(EINVAL, pfc_hostaddr_tostring(haddr, buf, sizeof(buf), 0));
    }

    for (uint32_t flags(1); flags != 0; flags <<= 1) {
        if (flags != PFC_HA2STR_TYPE) {
            ASSERT_EQ(EINVAL, pfc_hostaddr_tostring(&addr, buf, sizeof(buf),
                                                    flags));
        }
    }
}

/*
 * Error test case for pfc_hostaddr_gettype().
 */
TEST(hostaddr, gettype_error)
{
    ASSERT_EQ(AF_UNSPEC, pfc_hostaddr_gettype(NULL));

    pfc_hostaddr_t  addr;
    memset(&addr, 0, sizeof(addr));
    ASSERT_EQ(AF_UNSPEC, pfc_hostaddr_gettype(NULL));
}

/*
 * Error test case for pfc_hostaddr_getaddr().
 */
TEST(hostaddr, getaddr_error)
{
    struct in6_addr  iaddr;
    size_t  size(sizeof(iaddr));
    pfc_hostaddr_t  addr;
    memset(&addr, 0, sizeof(addr));

    ASSERT_EQ(EINVAL, pfc_hostaddr_getaddr(NULL, &iaddr, &size));
    ASSERT_EQ(EINVAL, pfc_hostaddr_getaddr(&addr, NULL, &size));
    ASSERT_EQ(EINVAL, pfc_hostaddr_getaddr(&addr, &iaddr, NULL));
    ASSERT_EQ(EINVAL, pfc_hostaddr_getaddr(&addr, &iaddr, &size));
}

/*
 * Test case for pfc_hostaddr_fromsockaddr().
 */
TEST(hostaddr, fromsockaddr)
{
    pfc_hostaddr_t  addr;

    /* AF_UNIX socket address. */
    struct sockaddr_un  un;
    un.sun_family = AF_UNIX;
    snprintf(un.sun_path, sizeof(un.sun_path), "/foo/bar/socket_addr");

    struct sockaddr *saddr(reinterpret_cast<struct sockaddr *>(&un));
    ASSERT_EQ(0, pfc_hostaddr_fromsockaddr(&addr, saddr));
    test_local(&addr);

    /* Error case. */
    ASSERT_EQ(EINVAL, pfc_hostaddr_fromsockaddr(NULL, saddr));
    ASSERT_EQ(EINVAL, pfc_hostaddr_fromsockaddr(&addr, NULL));

    for (int family(0); family <= 100; family++) {
        if (family == AF_UNSPEC || family == AF_UNIX || family == AF_INET ||
            family == AF_INET6) {
            continue;
        }

        un.sun_family = family;
        ASSERT_EQ(EAFNOSUPPORT, pfc_hostaddr_fromsockaddr(&addr, saddr));
    }

    RandomGenerator  rand;

    /* AF_INET4 socket address. */
    for (uint32_t loop(0); loop < 500; loop++) {
        struct sockaddr_in  sin;
        char  buf[PFC_HOSTADDR_STRSIZE_INET4];

        sin.sin_family = AF_INET;
        sin.sin_port = htons(80);
        random_inet4(rand, sin.sin_addr, buf, sizeof(buf));
        RETURN_ON_ERROR();

        saddr = reinterpret_cast<struct sockaddr *>(&sin);
        ASSERT_EQ(0, pfc_hostaddr_fromsockaddr(&addr, saddr));
        test_inet4(&addr, &sin.sin_addr, buf);
        RETURN_ON_ERROR();
    }

    /* AF_INET6 socket address. */
    for (uint32_t loop(0); loop < 500; loop++) {
        struct sockaddr_in6  sin6;
        char  buf[PFC_HOSTADDR_STRSIZE_INET6];

        sin6.sin6_family = AF_INET6;
        sin6.sin6_port = htons(80);
        sin6.sin6_flowinfo = 0;
        sin6.sin6_scope_id = 0;
        random_inet6(rand, sin6.sin6_addr, buf, sizeof(buf));
        RETURN_ON_ERROR();

        saddr = reinterpret_cast<struct sockaddr *>(&sin6);
        ASSERT_EQ(0, pfc_hostaddr_fromsockaddr(&addr, saddr));
        test_inet6(&addr, &sin6.sin6_addr, buf);
        RETURN_ON_ERROR();
    }

    /* AF_INET6 socket address with scope ID. */
    for (const uint32_t *scp(ipv6_scopes); scp < PFC_ARRAY_LIMIT(ipv6_scopes);
         scp++) {
        struct sockaddr_in6  sin6;
        char      buf[PFC_HOSTADDR_STRSIZE_INET6];
        uint32_t  scope(*scp);

        sin6.sin6_family = AF_INET6;
        sin6.sin6_port = htons(80);
        sin6.sin6_flowinfo = 0;
        sin6.sin6_scope_id = scope;
        random_inet6(rand, sin6.sin6_addr, buf, sizeof(buf));
        RETURN_ON_ERROR();

        std::string taddr(buf);
        snprintf(buf, sizeof(buf), "%u", scope);
        taddr.append("%");
        taddr.append(buf);
        saddr = reinterpret_cast<struct sockaddr *>(&sin6);
        ASSERT_EQ(0, pfc_hostaddr_fromsockaddr(&addr, saddr));
        test_inet6(&addr, &sin6.sin6_addr, taddr.c_str(), scope);
        RETURN_ON_ERROR();
    }
}

/*
 * Test case for pfc_hostaddr_compare().
 */
TEST(hostaddr, compare)
{
    const uint32_t  count(1000);
    TmpBuffer       tmpbuf(sizeof(pfc_hostaddr_t) * count);
    pfc_hostaddr_t  *array;
    TMPBUF_ASSERT(tmpbuf, array, pfc_hostaddr_t *);

    RandomGenerator  rand;
    pfc_hostaddr_t   prev_addr;

    for (uint32_t i(0); i < count; i++) {
        pfc_hostaddr_t  *addr(array + i);

        uint32_t c;
        RANDOM_INTEGER_MAX(rand, c, 5U);
        if (c == 0) {
            // Duplicate host address.
            *addr = prev_addr;
            if (pfc_hostaddr_gettype(addr) == AF_INET6) {
                RANDOM_INTEGER_MAX(rand, c, 2U);
                if (c == 0) {
                    // Change scope ID.
                    hostaddr_t  *hap(reinterpret_cast<hostaddr_t *>(addr));
                    hap->ha_scope++;
                }
                continue;
            }
        }

        RANDOM_INTEGER_MAX(rand, c, 5U);
        if (c == 0) {
            ASSERT_EQ(0, pfc_hostaddr_init_local(addr));
        }
        else if (c <= 3) {
            struct in_addr  iaddr;
            char  buf[PFC_HOSTADDR_STRSIZE_INET4];

            random_inet4(rand, iaddr, buf, sizeof(buf));
            RETURN_ON_ERROR();

            ASSERT_EQ(0, pfc_hostaddr_init(addr, AF_INET, buf));
        }
        else {
            struct in6_addr  iaddr;
            char  buf[PFC_HOSTADDR_STRSIZE_INET6];

            random_inet6(rand, iaddr, buf, sizeof(buf));
            RETURN_ON_ERROR();

            const char  *taddr(buf);
            std::string tstr;
            uint32_t    scope;
            RANDOM_INTEGER_MAX(rand, scope, 5U);
            if (scope != 0) {
                tstr.assign(taddr);
                tstr.append("%");

                snprintf(buf, sizeof(buf), "%u", scope);
                tstr.append(buf);
                taddr = tstr.c_str();
            }

            ASSERT_EQ(0, pfc_hostaddr_init(addr, AF_INET6, taddr));
        }

        prev_addr = *addr;
    }

    /* Try to sort pfc_hostaddr_t array using qsort(3). */
    qsort(array, count, sizeof(pfc_hostaddr_t),
          (int (*)(const void *, const void *))pfc_hostaddr_compare);

    /* Ensure that pfc_hostaddr_t elements are sorted as appropriate. */
    int      prev_family(AF_UNSPEC);
    uint32_t prev_inet4(0);
    uint64_t prev_inet6[2] = {0U, 0U};
    uint64_t prev_scope = 0;

    for (uint32_t i(0); i < count; i++) {
        pfc_hostaddr_t  *addr(array + i);

        int  family(pfc_hostaddr_gettype(addr));
        ASSERT_NE(AF_UNSPEC, family);

        /* AF_UNIX must be less than other types. */
        if (family == AF_UNIX) {
            if (prev_family == AF_UNSPEC) {
                prev_family = family;
            }
            else {
                ASSERT_EQ(AF_UNIX, prev_family);
            }
            continue;
        }

        /* AF_INET must be less than AF_INET6, and be greater than AF_UNIX. */
        if (family == AF_INET) {
            struct in_addr  iaddr;
            size_t size(sizeof(iaddr));
            ASSERT_EQ(0, pfc_hostaddr_getaddr(addr, &iaddr, &size));
            ASSERT_EQ(sizeof(iaddr), size);

            uint32_t  inet4(ntohl(iaddr.s_addr));

            if (prev_family == AF_UNSPEC || prev_family == AF_UNIX) {
                prev_family = family;
            }
            else {
                ASSERT_EQ(AF_INET, prev_family);
                ASSERT_LE(prev_inet4, inet4);
            }
            prev_inet4 = inet4;
            continue;
        }

        /* AF_INET6 must be greater than other types. */
        struct in6_addr  iaddr;
        size_t size(sizeof(iaddr));
        ASSERT_EQ(0, pfc_hostaddr_getaddr(addr, &iaddr, &size));
        ASSERT_EQ(sizeof(iaddr), size);

        uint64_t  inet6[2];
        memcpy(inet6, &iaddr, sizeof(inet6));
        inet6[0] = ntohll(inet6[0]);
        inet6[1] = ntohll(inet6[1]);

        if (prev_family == AF_UNSPEC || prev_family == AF_UNIX ||
            prev_family == AF_INET) {
            prev_family = family;
        }
        else {
            ASSERT_EQ(AF_INET6, prev_family);
            if (prev_inet6[0] == inet6[0]) {
                if (prev_inet6[1] == inet6[1]) {
                    uint32_t scope;
                    ASSERT_EQ(0, pfc_hostaddr_getscope(addr, &scope));
                    ASSERT_LE(prev_scope, scope);
                    prev_scope = scope;
                }
                else {
                    ASSERT_LT(prev_inet6[1], inet6[1]);
                    prev_scope = 0;
                }
            }
            else {
                ASSERT_LT(prev_inet6[0], inet6[0]);
                prev_scope = 0;
            }
        }

        prev_inet6[0] = inet6[0];
        prev_inet6[1] = inet6[1];
    }
}

/*
 * Test case for pfc_sockaddr_init() with local host address.
 */
TEST(hostaddr, sockaddr_local)
{
    pfc_hostaddr_t  haddr;

    ASSERT_EQ(0, pfc_hostaddr_init_local(&haddr));

    SockAddr    sockaddr;

    // NULL hint.
    {
        const char  *path("/foo/bar/socket_path");

        ASSERT_EQ(0, sockaddr.init(haddr, path, NULL));
        const pfc_sockaddr_t  *sap(sockaddr.get());
        ASSERT_TRUE(sap->sa_addr != NULL);

        const struct sockaddr_un  *saddr
            (reinterpret_cast<const struct sockaddr_un *>(sap->sa_addr));
        ASSERT_EQ(AF_UNIX, saddr->sun_family);
        ASSERT_STREQ(path, saddr->sun_path);
        ASSERT_EQ(PFC_SOCK_UNADDR_SIZE(strlen(path)), sap->sa_addrlen);
        ASSERT_EQ(SOCK_STREAM, sap->sa_socktype);
        ASSERT_EQ(0, sap->sa_protocol);
    }

    /* Specify socket type. */
    {
        const char *path("/test/socket/path");
        size_t     pathlen(strlen(path));
        for (int socktype(1); socktype <= 32; socktype++) {
            struct addrinfo  hints;
            memset(&hints, 0, sizeof(hints));
            hints.ai_socktype = socktype;

            int  err(sockaddr.init(haddr, path, &hints));

            if (IS_SOCKTYPE_VALID_UN(socktype)) {
                ASSERT_EQ(0, err);
                const pfc_sockaddr_t  *sap(sockaddr.get());
                ASSERT_TRUE(sap->sa_addr != NULL);

                const struct sockaddr_un  *saddr
                    (reinterpret_cast<const struct sockaddr_un *>
                     (sap->sa_addr));
                ASSERT_EQ(AF_UNIX, saddr->sun_family);
                ASSERT_STREQ(path, saddr->sun_path);
                ASSERT_EQ(PFC_SOCK_UNADDR_SIZE(pathlen), sap->sa_addrlen);
                ASSERT_EQ(socktype, sap->sa_socktype);
                ASSERT_EQ(0, sap->sa_protocol);
            }
            else {
                ASSERT_EQ(EAI_SOCKTYPE, err);
            }
        }
    }

    /* Long path name. */
    {
        char path[120];
        const struct sockaddr_un  *saddr;
        for (size_t len(100); len <= sizeof(saddr->sun_path); len++) {
            memset(path, 'a', len);
            path[len] = '\0';

            ASSERT_EQ(0, sockaddr.init(haddr, path, NULL));
            const pfc_sockaddr_t  *sap(sockaddr.get());
            ASSERT_TRUE(sap->sa_addr != NULL);

            saddr = reinterpret_cast<const struct sockaddr_un *>(sap->sa_addr);
            ASSERT_EQ(AF_UNIX, saddr->sun_family);
            ASSERT_STREQ(path, saddr->sun_path);
            ASSERT_EQ(PFC_SOCK_UNADDR_SIZE(len), sap->sa_addrlen);
            ASSERT_EQ(SOCK_STREAM, sap->sa_socktype);
            ASSERT_EQ(0, sap->sa_protocol);
        }

        for (size_t len(sizeof(saddr->sun_path) + 1); len < sizeof(path);
             len++) {
            memset(path, 'a', len);
            path[len] = '\0';

            errno = 0;
            ASSERT_EQ(EAI_SYSTEM, sockaddr.init(haddr, path, NULL));
            ASSERT_EQ(ENAMETOOLONG, errno);
        }
    }

    /* Invalid service.*/
    ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, NULL, NULL));
    ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, "", NULL));
}

/*
 * Test case for pfc_sockaddr_init() with IPv4 host address.
 */
TEST(hostaddr, sockaddr_inet4)
{
    RandomGenerator rand;
    struct in_addr  iaddr;
    char            buf[PFC_HOSTADDR_STRSIZE_INET4];
    pfc_hostaddr_t  haddr;
    random_inet4(rand, iaddr, buf, sizeof(buf));
    RETURN_ON_ERROR();

    ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET, buf));

    SockAddr    sockaddr;

    // NULL hint.
    {
        // "shell" must be 514/tcp.
        ASSERT_EQ(0, sockaddr.init(haddr, "shell", NULL));
        test_sockaddr_in(sockaddr, iaddr, 514, SOCK_STREAM);
        RETURN_ON_ERROR();

        // NULL service name.
        ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, NULL, NULL));

        // Empty service name.
        ASSERT_EQ(0, sockaddr.init(haddr, "", NULL));
        test_sockaddr_in(sockaddr, iaddr, 0, 0);
        RETURN_ON_ERROR();

        // Numeric port number.
        for (uint32_t loop(0); loop < 500; loop++) {
            uint32_t  port;
            RANDOM_INTEGER_MAX(rand, port, 65536U);

            random_inet4(rand, iaddr, buf, sizeof(buf));
            RETURN_ON_ERROR();
            ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET, buf));

            char  sport[32];
            snprintf(sport, sizeof(sport), "%u", port);
            ASSERT_EQ(0, sockaddr.init(haddr, sport, NULL));
            test_sockaddr_in(sockaddr, iaddr, port, 0);
            RETURN_ON_ERROR();
        }
    }

    // Specifying hint.
    struct addrinfo  hints;
    for (uint32_t loop(0); loop < 500; loop++) {
        memset(&hints, 0, sizeof(hints));

        uint32_t  t;
        RANDOM_INTEGER(rand, t);
        if (t & 0x1) {
            hints.ai_socktype = SOCK_DGRAM;
        }
        else {
            hints.ai_socktype = SOCK_STREAM;
        }
        if (t & 0x2) {
            hints.ai_protocol = (hints.ai_socktype == SOCK_STREAM)
                ? IPPROTO_TCP : IPPROTO_UDP;
        }

        random_inet4(rand, iaddr, buf, sizeof(buf));
        RETURN_ON_ERROR();
        ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET, buf));

        uint32_t  port;
        RANDOM_INTEGER_MAX(rand, port, 65536U);

        char  sport[32];
        snprintf(sport, sizeof(sport), "%u", port);
        hints.ai_flags = AI_NUMERICSERV;

        ASSERT_EQ(0, sockaddr.init(haddr, sport, &hints));
        test_sockaddr_in(sockaddr, iaddr, port, hints.ai_socktype);
        RETURN_ON_ERROR();

        // NULL service name.
        ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, NULL, &hints));

        // Empty service name.
        ASSERT_EQ(0, sockaddr.init(haddr, "", &hints));
        test_sockaddr_in(sockaddr, iaddr, 0, hints.ai_socktype);
        RETURN_ON_ERROR();
    }

    // Raw socket.
    {
        random_inet4(rand, iaddr, buf, sizeof(buf));
        RETURN_ON_ERROR();
        ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET, buf));

        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_RAW;
        ASSERT_EQ(EAI_SERVICE, sockaddr.init(haddr, "telnet", &hints));
        ASSERT_EQ(0, sockaddr.init(haddr, NULL, &hints));
        test_sockaddr_in(sockaddr, iaddr, 0, SOCK_RAW);
        RETURN_ON_ERROR();
    }

    // Bad service name.
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICSERV;
    ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, "telnet", &hints));
    ASSERT_EQ(EAI_SERVICE, sockaddr.init(haddr, "unknown-service-0123", NULL));

    hints.ai_socktype = SOCK_STREAM;
    ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, NULL, &hints));
    hints.ai_socktype = SOCK_DGRAM;
    ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, NULL, &hints));

    // Socket type mismatch.
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    ASSERT_EQ(EAI_SERVICE, sockaddr.init(haddr, "shell", &hints));

    // Protocol type mismatch.
    memset(&hints, 0, sizeof(hints));
    hints.ai_protocol = IPPROTO_UDP;
    ASSERT_EQ(EAI_SERVICE, sockaddr.init(haddr, "shell", &hints));

    // Unsuported socket type.
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_UDP;
    ASSERT_EQ(EAI_SOCKTYPE, sockaddr.init(haddr, "10", &hints));
}

/*
 * Test case for pfc_sockaddr_init() with IPv6 host address.
 */
TEST(hostaddr, sockaddr_inet6)
{
    RandomGenerator rand;
    struct in6_addr iaddr;
    char            buf[PFC_HOSTADDR_STRSIZE_INET6];
    pfc_hostaddr_t  haddr;
    random_inet6(rand, iaddr, buf, sizeof(buf));
    RETURN_ON_ERROR();

    ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET6, buf));

    SockAddr    sockaddr;

    // NULL hint.
    {
        // "syslog" must be 514/udp.
        ASSERT_EQ(0, sockaddr.init(haddr, "syslog", NULL));
        test_sockaddr_in6(sockaddr, iaddr, 514, SOCK_DGRAM);
        RETURN_ON_ERROR();

        // NULL service name.
        ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, NULL, NULL));

        // Empty service name.
        ASSERT_EQ(0, sockaddr.init(haddr, "", NULL));
        test_sockaddr_in6(sockaddr, iaddr, 0, 0);
        RETURN_ON_ERROR();

        // Numeric port number.
        for (uint32_t loop(0); loop < 500; loop++) {
            uint32_t  port;
            RANDOM_INTEGER_MAX(rand, port, 65536U);

            random_inet6(rand, iaddr, buf, sizeof(buf));
            RETURN_ON_ERROR();
            ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET6, buf));

            char  sport[32];
            snprintf(sport, sizeof(sport), "%u", port);
            ASSERT_EQ(0, sockaddr.init(haddr, sport, NULL));
            test_sockaddr_in6(sockaddr, iaddr, port, 0);
            RETURN_ON_ERROR();
        }
    }

    // Specifying hint.
    struct addrinfo  hints;
    for (uint32_t loop(0); loop < 500; loop++) {
        memset(&hints, 0, sizeof(hints));

        uint32_t  t;
        RANDOM_INTEGER(rand, t);
        if (t & 1) {
            hints.ai_socktype = SOCK_DGRAM;
        }
        else {
            hints.ai_socktype = SOCK_STREAM;
        }
        if (t & 0x2) {
            hints.ai_protocol = (hints.ai_socktype == SOCK_STREAM)
                ? IPPROTO_TCP : IPPROTO_UDP;
        }

        random_inet6(rand, iaddr, buf, sizeof(buf));
        RETURN_ON_ERROR();
        ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET6, buf));

        uint32_t  port;
        RANDOM_INTEGER_MAX(rand, port, 65536U);

        char  sport[32];
        snprintf(sport, sizeof(sport), "%u", port);
        hints.ai_flags = AI_NUMERICSERV;

        ASSERT_EQ(0, sockaddr.init(haddr, sport, &hints));
        test_sockaddr_in6(sockaddr, iaddr, port, hints.ai_socktype);
        RETURN_ON_ERROR();

        // NULL service name.
        ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, NULL, &hints));

        // Empty service name.
        ASSERT_EQ(0, sockaddr.init(haddr, "", &hints));
        test_sockaddr_in6(sockaddr, iaddr, 0, hints.ai_socktype);
        RETURN_ON_ERROR();
    }

    // Specify IPv6 scope ID.
    for (const uint32_t *scp(ipv6_scopes); scp < PFC_ARRAY_LIMIT(ipv6_scopes);
         scp++) {
        uint32_t     scope(*scp);
        const char   *v6addr("1111:2222:3333:4444:5555::");
        std::string  textaddr(v6addr);

        struct in6_addr  iaddr;
        ASSERT_EQ(1, inet_pton(AF_INET6, v6addr, &iaddr));

        char  buf[32];
        snprintf(buf, sizeof(buf), "%u", scope);
        textaddr.append("%");
        textaddr.append(buf);
        ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET6, textaddr.c_str()));

        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        ASSERT_EQ(0, sockaddr.init(haddr, "30", &hints));
        test_sockaddr_in6(sockaddr, iaddr, 30, hints.ai_socktype, scope);
        RETURN_ON_ERROR();
    }

    // Raw socket.
    {
        random_inet6(rand, iaddr, buf, sizeof(buf));
        RETURN_ON_ERROR();
        ASSERT_EQ(0, pfc_hostaddr_init(&haddr, AF_INET6, buf));

        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_RAW;
        ASSERT_EQ(EAI_SERVICE, sockaddr.init(haddr, "telnet", &hints));
        ASSERT_EQ(0, sockaddr.init(haddr, NULL, &hints));
        test_sockaddr_in6(sockaddr, iaddr, 0, SOCK_RAW);
        RETURN_ON_ERROR();
    }

    // Bad service name.
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICSERV;
    ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, "telnet", &hints));
    ASSERT_EQ(EAI_SERVICE, sockaddr.init(haddr, "unknown-service-0123", NULL));

    hints.ai_socktype = SOCK_STREAM;
    ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, NULL, &hints));
    hints.ai_socktype = SOCK_DGRAM;
    ASSERT_EQ(EAI_NONAME, sockaddr.init(haddr, NULL, &hints));

    // Socket type mismatch.
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    ASSERT_EQ(EAI_SERVICE, sockaddr.init(haddr, "syslog", &hints));

    // Protocol type mismatch.
    memset(&hints, 0, sizeof(hints));
    hints.ai_protocol = IPPROTO_TCP;
    ASSERT_EQ(EAI_SERVICE, sockaddr.init(haddr, "syslog", &hints));

    // Unsuported socket type.
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_TCP;
    ASSERT_EQ(EAI_SOCKTYPE, sockaddr.init(haddr, "10", &hints));
}

/*
 * Error test case for pfc_sockaddr_init()
 */
TEST(hostaddr, sockaddr_error)
{
    pfc_sockaddr_t  sa;
    pfc_hostaddr_t  ha;

    errno = 0;
    ASSERT_EQ(EAI_SYSTEM, pfc_sockaddr_init(NULL, &ha, "service", NULL));
    ASSERT_EQ(EINVAL, errno);

    errno = 0;
    ASSERT_EQ(EAI_SYSTEM, pfc_sockaddr_init(&sa, NULL, "service", NULL));
    ASSERT_EQ(EINVAL, errno);

    memset(&ha, 0, sizeof(ha));
    errno = 0;
    ASSERT_EQ(EAI_SYSTEM, pfc_sockaddr_init(&sa, &ha, "service", NULL));
    ASSERT_EQ(EINVAL, errno);
}
