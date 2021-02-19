########################

#import packages

import re
import sys
import socket
import ssl


#constants
addr_fam = socket.AF_INET
sock_type = socket.SOCK_STREAM

#########functions#############


def send_message(host,port,use_ssl,message):
    sock = socket.socket(addr_fam,sock_type)
    if not use_ssl:
        sock.connect((host,port))
        sock.send(message.encode()) #send request #need to encode
        data = sock.recv(10000) #get response
    else:
        context = ssl.create_default_context()
        ssl_sock = context.wrap_socket(sock,server_hostname = host)
        ssl_sock.connect((host,port))
        ssl_sock.send(message.encode()) #send request
        data = ssl_sock.recv(10000) #get response
        ssl_sock.close()
    sock.close()
    data = data.decode(encoding='utf-8',errors='ignore')
    return data

def connect_http2(host,port):
    sock = socket.socket(addr_fam,sock_type)
    context = ssl.create_default_context()
    context.set_alpn_protocols(['h2']) #set to HTTP2 only
    ssl_sock = context.wrap_socket(sock,server_hostname = host)
    sock = socket.socket(addr_fam,sock_type)
    ssl_sock.connect((host,port))

    prot_used = ssl_sock.selected_alpn_protocol()
    print("Protocol used: {}".format(prot_used))
    print()
    print()

    if prot_used == 'h2':
        support_http2 = True
    else:
        support_http2 = False
    return support_http2

#####parsing functions

def get_info(data):
    version = None
    if data:
        version = re.search("(?<=HTTP/)\d\.\d",data)
    if version:
        version = version[0]
    else:
        version = None
    status_code = None
    if data:
        status_code = re.search("(?<=\s)\d\d\d(?=\s)",data)
    if status_code:
        status_code = status_code[0]
    else:
        status_code = None
    return version,status_code


def get_cookies(data):
    cookie_list = re.findall("[Ss]et-[Cc]ookie.*?(?=\n)",data) # * needs to be non greedy #get full line first
    cookie_info_list = []
    for cookie in cookie_list:
        cookie_name = re.search("(?<=[Ss]et-[Cc]ookie:\s).*?(?==)",cookie)
        cookie_expire = re.search("(?<=[Ee]xpires=).*?(?=[;\n])",cookie)
        cookie_domain = re.search("(?<=[Dd]omain=).*?(?=[;\n])",cookie)
        if cookie_name:
            cookie_name = cookie_name[0]
        if cookie_expire:
            cookie_expire = cookie_expire[0]
        if cookie_domain:
            cookie_domain = cookie_domain[0]
        cookie_info_list.append((cookie_name,cookie_expire,cookie_domain))
    return cookie_info_list

def process_status_code(status_code):
    if not status_code:
        return False
    if status_code[0] in ['2','3']:
        return True
    return False

def print_results(host,support_https,support_http11,support_http2,cookie_list_master):
    print("--- RESULTS ---")
    print("website: {}".format(host))
    print("1. Supports HTTPS: {}".format(support_https))
    print("2. Supports HTTP1.1: {}".format(support_http11))
    print("3. Supports HTTP2: {}".format(support_http2))
    print("4. List of Cookies:")
    for cookie in cookie_list_master:
        print("cookie name: {}".format(cookie[0]),end="")
        if cookie[1]:
            print("; expires: {}".format(cookie[1]),end="")
        if cookie[2]:
            print("; domain: {}".format(cookie[2]),end="")
        print()
    print()


def print_int_results(message,data,error,error_msg):
    print("--- REQUEST ---")
    print(message)
    if error:
        print("ERROR connecting or sending message!!")
        print("Error message:")
        print(error_msg)
        print()
        print()
    print("--- RESPONSE ---")
    print(data)
    version,code = get_info(data)
    print()
    print("--- EXTRACTED DATA FROM RESPONSE ---")
    print("HTTP version: {}".format(version))
    print("Status code: {}".format(code))
    print()
    print()

def main():
    ################# supports booleans: ############

    support_11 = False
    support_s = False
    support_2 = False


    ########### errors ###########################

    error_http11 = False
    error_https10 = False
    error_https11 = False
    error_http2 = False

    ######### parse args (get URI) ######################

    host = sys.argv[1]

    print("--- INPUT ---")
    print(host) #to check
    print()


    ##################  send connections and generate data: #####################
    print("Connecting with HTTP/1.1...")
    print()

    port = 80
    version = "1.1"
    message = "GET /index.html HTTP/{}\r\nHost:{}\r\n\r\n".format(version,host) #changing message to be like example
    error_msg = None
    try:
        data_11 = send_message(host,port,False,message)
        err = None
    except Exception as err:
        error_http11 = True
        data_11 = None
        error_msg = err
    print_int_results(message,data_11,error_http11,error_msg)
    
    print("Connecting with HTTPS/1.0...")
    print()

    port = 443
    version = "1.0"
    message = "GET /index.html HTTP/{}\r\nHost:{}\r\n\r\n".format(version,host) #changing message to be like example
    error_msg = None
    try:
        data_s10 = send_message(host,port,True,message)
    except Exception as err:
        error_https10 = True
        data_s10 = None
        error_msg = err
        print(error_msg)
    print_int_results(message,data_s10,error_https10,error_msg)

    print("Connecting with HTTPS/1.1...")
    print()

    version = "1.1"
    message = "GET /index.html HTTP/{}\r\nHost:{}\r\n\r\n".format(version,host) #changing message to be like example
    error_msg = None
    try:
        data_s11 = send_message(host,port,True,message)
    except Exception as err:
        error_https11 = True
        data_s11 = None
        error_msg = err
    print_int_results(message,data_s11,error_https11,error_msg)

    print("Connecting with HTTP/2...")
    print()

    error_msg = None
    try:
        support_2 = connect_http2(host,port)
    except Exception as err:
        print("Error connecting!!")
        print("Error message:")
        error_msg = err
        print(error_msg)
        print()
        print()
        support_2 = False

    ######################now print intermediate results ########################## 

    if data_11:
        version_11,status_code_11 = get_info(data_11)

    #######################  parsing data ###########

    #Assume http/1.1 supported if for at least one of the http/1.1 connection attempts (with or without ssl)
    #there was no error thrown and the version 1.1 was given in the response.
    if not error_http11:
        version,status_code = get_info(data_11)
        if version == "1.1":
            support_11 = True
        else:
            if not error_https11:
                version,status_code = get_info(data_s11)
                if version == "1.1":
                    support_11 = True
    #otherwise will be False.
    
    #Assume https is supported if for at least one of the https connection attempts 
    #there was no error thrown and an http version was given.
    if not error_https10 or not error_https11:
        if data_s10:
            version,status_code = get_info(data_s10)
            if version:
                support_s = True
        if data_s11:
            version,status_code = get_info(data_s11)
            if version:
                support_s = True
            
        #assuming only reason https not supported is if error thrown. May also 
        #maybe should check that a status code and version are given.


    #now collect cookies.
    data_list = [data_11,data_s10,data_s11]
    cookie_list_master = []
    for data in data_list:
        if data: #don't use data if it is None
            cookie_list = get_cookies(data)
            for cookie in cookie_list:
                if cookie not in cookie_list_master:
                    cookie_list_master.append(cookie)

    print_results(host,support_s,support_11,support_2,cookie_list_master)


if __name__=='__main__':
    main()
