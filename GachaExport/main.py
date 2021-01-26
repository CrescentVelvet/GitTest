import json
import requests
import urllib.parse
import sys

# url = "https://hk4e-api.mihoyo.com/event/gacha_info/  \
# api/getGachaLog?authkey_ver=1&sign_type=2&auth_appid= \
# webview_gacha&init_type=301&gacha_id=2ffa459718702872a\
# 52867fa0521e32b6843b0&lang=zh-cn&device_type=pc&ext=%7\
# b%22loc%22%3a%7b%22x%22%3a2233.840576171875%2c%22y%22%\
# 3a212.8805389404297%2c%22z%22%3a-915.6687622070313%7d%\
# 2c%22platform%22%3a%22WinST%22%7d&game_version=CNRELWi\
# n1.2.0_R1771533_S1847412_D1816310&region=cn_gf01&authk\
# ey=ha5uTCJKQcBhkUyflLcEn6dQPGoMmJqHgTTdR%2fd3jL%2bIlhO\
# ZcRSaD1Slc2MEy4q3tVlW2lE2d%2fMCoSoZ1PUWkwPKfTy%2f08lcU\
# h2yRyI071P0OvDjHN57ixrRPdqdYYOWrT8q3pB9ueuw84Oiwuk%2fT\
# 4Nig7Vftpfl0buzhPMgXhkNrNJsRHx%2fUcuGZqTXJn3WZ%2f%2bsr\
# xtRHconxGr%2bkd1Gxvwe7AfahdmMzBASSDsvcQaadd8USKRQmQBmj\
# ZkGRd78DaUc9XugWZTvdlQW20mJCKDYlSuYDNctOOObYYDTJ6DEisL\
# Xm3eQCc0PAAobMYGc%2bk2TQIB08faBMX9gDOVz4wr6nFY6%2bmtBK\
# ilU9sdl8Z2Q%2fMAqT%2bT%2bH3feJOFCbbbeO2UEcedt06WcoOyr1\
# wRHkjz6xt8%2fUwcCl%2fDsD9ORzgzk1BOGdXImd4UuktPxj%2fNbG\
# p5IBZnpbHLxVt%2bGGC4%2foCIKbiRpBbuG6HVn2rYqnGkXyDg7YpA\
# f2xcRjTZ%2fD0bfVUEtSaiGLxe8%2blE8VCOHrUzQ1GR7AYZiupyx5\
# PWudvzeoHnd6sYJjhNtp5Lc7LO74hmORKGvE8TYRKDs%2b%2b3ykCf\
# VlmkrSu3mfjZTOObMS0fKl3RJn4lVma1McItDfvCUeU4vhv28fPncs\
# 9SBqT%2btSvE%2fPpF0oV2TEwoMErWyCnrYhsZYFVHNkB4NM9kvRMx\
# urkBaor9aznkOePq8%2fpwvH5me01aJ9IU%2b8DIcDAlna%2bp7Dpz\
# ZICy%2bFFLokyNbJFYuo5HydxTU0WB1HbyMiQYVab1x1qS7%2bASqR\
# dEkJeeL07e1D3Cx8LOzpbosmQ4%2fPBKt4OvfDpTmUrDMrv3oWRZyF\
# HUwJF%2bIV7YTVuIwVGtTMrugkcI70hkbucbzFXBxDwhjKqVxPPWNn\
# BiK9nSe3Q5pnvQCMiwDQpZTDZ%2b%2b5HZ03AvLbSAnByC0qk7Mlni\
# zeKnuqIk321fQGcExXwOvI8K8cSEtqQjXUG2gO%2fd7kV%2bgG%2f6\
# lYGWZEbRwQeSHJugMEtzy&game_biz=hk4e_cn&gacha_type=301&\
# page=1&size=6"

url = "https://hk4e-api.mihoyo.com/event/gacha_info/api/getGachaLog?authkey_ver=1&sign_type=2&auth_appid=webview_gacha&init_type=301&gacha_id=2ffa459718702872a52867fa0521e32b6843b0&lang=zh-cn&device_type=pc&ext=%7b%22loc%22%3a%7b%22x%22%3a2233.840576171875%2c%22y%22%3a212.8805389404297%2c%22z%22%3a-915.6687622070313%7d%2c%22platform%22%3a%22WinST%22%7d&game_version=CNRELWin1.2.0_R1771533_S1847412_D1816310&region=cn_gf01&authkey=ha5uTCJKQcBhkUyflLcEn6dQPGoMmJqHgTTdR%2fd3jL%2bIlhOZcRSaD1Slc2MEy4q3tVlW2lE2d%2fMCoSoZ1PUWkwPKfTy%2f08lcUh2yRyI071P0OvDjHN57ixrRPdqdYYOWrT8q3pB9ueuw84Oiwuk%2fT4Nig7Vftpfl0buzhPMgXhkNrNJsRHx%2fUcuGZqTXJn3WZ%2f%2bsrxtRHconxGr%2bkd1Gxvwe7AfahdmMzBASSDsvcQaadd8USKRQmQBmjZkGRd78DaUc9XugWZTvdlQW20mJCKDYlSuYDNctOOObYYDTJ6DEisLXm3eQCc0PAAobMYGc%2bk2TQIB08faBMX9gDOVz4wr6nFY6%2bmtBKilU9sdl8Z2Q%2fMAqT%2bT%2bH3feJOFCbbbeO2UEcedt06WcoOyr1wRHkjz6xt8%2fUwcCl%2fDsD9ORzgzk1BOGdXImd4UuktPxj%2fNbGp5IBZnpbHLxVt%2bGGC4%2foCIKbiRpBbuG6HVn2rYqnGkXyDg7YpAf2xcRjTZ%2fD0bfVUEtSaiGLxe8%2blE8VCOHrUzQ1GR7AYZiupyx5PWudvzeoHnd6sYJjhNtp5Lc7LO74hmORKGvE8TYRKDs%2b%2b3ykCfVlmkrSu3mfjZTOObMS0fKl3RJn4lVma1McItDfvCUeU4vhv28fPncs9SBqT%2btSvE%2fPpF0oV2TEwoMErWyCnrYhsZYFVHNkB4NM9kvRMxurkBaor9aznkOePq8%2fpwvH5me01aJ9IU%2b8DIcDAlna%2bp7DpzZICy%2bFFLokyNbJFYuo5HydxTU0WB1HbyMiQYVab1x1qS7%2bASqRdEkJeeL07e1D3Cx8LOzpbosmQ4%2fPBKt4OvfDpTmUrDMrv3oWRZyFHUwJF%2bIV7YTVuIwVGtTMrugkcI70hkbucbzFXBxDwhjKqVxPPWNnBiK9nSe3Q5pnvQCMiwDQpZTDZ%2b%2b5HZ03AvLbSAnByC0qk7MlnizeKnuqIk321fQGcExXwOvI8K8cSEtqQjXUG2gO%2fd7kV%2bgG%2f6lYGWZEbRwQeSHJugMEtzy&game_biz=hk4e_cn&gacha_type=301&page=1&size=6"


FLAG_WRITE_CSV = 1
# 是否写入CSV
FLAG_WRITE_XLS = 0
# 是否写入EXCEL
FLAG_SHOW_DETAIL = 0
# 是否展示详情
FLAG_CLEAN = 1
# 是否清除历史文件


def main():

    print("检查URL", end="...", flush=True)
    checkApi(url)
    print("合法")

    print("获取物品信息", end="...", flush=True)
    gachaInfo = initGachaInfo()
    print("物品数：" + str(len(gachaInfo)))

    print("获取卡池信息", end="...", flush=True)
    gachaTypeIds, gachaTypeNames, gachaTypeDict = initGachaTypes()
    print(" ".join(gachaTypeNames))

    print("获取抽卡数据", end="...", flush=True)
    gachaLists = []
    for gachaTypeId in gachaTypeIds:
        if FLAG_SHOW_DETAIL:
            print(gachaTypeDict[gachaTypeId])
        gachaList = getGachaList(gachaInfo, gachaTypeId)
        gachaLists.append(gachaList)
        if not FLAG_SHOW_DETAIL:
            print(gachaTypeDict[gachaTypeId], end=" ", flush=True)
    print("")

    if FLAG_CLEAN:
        print("清除历史文件", end="...", flush=True)
        import os

        del_paths = [name for name in os.listdir(sys.path[0]) if name.startswith("gacha") and (name.endswith(".csv") or name.endswith(".xlsx"))]
        for del_path in del_paths:
            try:
                os.remove(sys.path[0] + "\\" + del_path)
                print(del_path, end=" ", flush=True)
            except:
                pass
        print("")
        
    print("写入文件", end="...", flush=True)
    if FLAG_WRITE_CSV:
        writeCSV(gachaLists, gachaTypeIds)
        print("CSV", end=" ", flush=True)

    if FLAG_WRITE_XLS:
        writeXLSX(gachaLists, gachaTypeNames)
        print("XLS", end=" ", flush=True)


def getApi(gachaType, size, page):
    parsed = urllib.parse.urlparse(url)
    querys = urllib.parse.parse_qsl(parsed.query)
    param_dict = dict(querys)
    param_dict["size"] = size
    param_dict["gacha_type"] = gachaType
    param_dict["page"] = page
    param = urllib.parse.urlencode(param_dict)
    path = url.split("?")[0]
    api = path + "?" + param
    return api


def getInfo(item_id, gachaInfo):
    for info in gachaInfo:
        if info["item_id"] == item_id:
            return info["name"] + "," + info["item_type"] + "," + info["rank_type"]
    return "物品ID未找到"


def checkApi(url):
    if not url:
        print("未填入url")
        exit()
    if "getGachaLog" not in url:
        print("错误的url，检查是否包含getGachaLog")
        exit()
    try:
        requests.packages.urllib3.disable_warnings()
        r = requests.get(url, verify=False)
        s = r.content.decode("utf-8")
        j = json.loads(s)
    except Exception as e:
        print("API请求解析出错：" + str(e))
        exit()

    if not j["data"]:
        if j["message"] == "authkey valid error":
            print("authkey错误，请重新抓包获取url")
        else:
            print("数据为空，错误代码：" + j["message"])
        exit()
    return url


def getQueryVariable(variable):
    query = url.split("?")[1]
    vars = query.split("&")
    for v in vars:
        if v.split("=")[0] == variable:
            return v.split("=")[1]
    return ""


def initGachaInfo():
    requests.packages.urllib3.disable_warnings()
    region = getQueryVariable("region")
    lang = getQueryVariable("lang")
    gachaInfoUrl = "https://webstatic.mihoyo.com/hk4e/gacha_info/{}/items/{}.json".format(region, lang)
    r = requests.get(gachaInfoUrl, verify=False)
    s = r.content.decode("utf-8")
    gachaInfo = json.loads(s)
    return gachaInfo


def initGachaTypes():
    requests.packages.urllib3.disable_warnings()
    r = requests.get(url.replace("getGachaLog", "getConfigList"), verify=False)
    s = r.content.decode("utf-8")
    configList = json.loads(s)
    gachaTypeLists = []
    for gachaType in configList["data"]["gacha_type_list"]:
        gachaTypeLists.append([gachaType["key"], gachaType["name"]])
    gachaTypeLists.sort()
    gachaTypeDict = dict(gachaTypeLists)
    gachaTypeIds = []
    gachaTypeNames = []
    for gachaTypeId, gachaTypeName in gachaTypeLists:
        gachaTypeIds.append(gachaTypeId)
        gachaTypeNames.append(gachaTypeName)
    return gachaTypeIds, gachaTypeNames, gachaTypeDict


def getGachaList(gachaInfo, gachaTypeId):
    requests.packages.urllib3.disable_warnings()
    size = "20"
    # api限制一页最大20
    gachaList = []
    for page in range(1, 9999):
        api = getApi(gachaTypeId, size, page)
        r = requests.get(api, verify=False)
        s = r.content.decode("utf-8")
        j = json.loads(s)

        gacha = j["data"]["list"]
        if not len(gacha):
            break
        for i in gacha:
            time = i["time"]
            item_id = i["item_id"]
            info = time + "," + item_id + "," + getInfo(item_id, gachaInfo)
            gachaList.append(info)
            if FLAG_SHOW_DETAIL:
                print(info)
    return gachaList


def writeCSV(gachaLists, gachaTypes):
    for id in range(len(gachaTypes)):
        # filename = f"{sys.path[0]}\\gacha{gachaTypes[id]}.csv"
        # filename = "".format(str(sys.path[0])).format("gecha-").format(str(gachaTypes[id])).format(".csv")
        filename = "gecha".format(gachaTypes[id])
        print("gecha".format(str(gachaTypes[id])))
        f = open(filename, "w", encoding="utf-8-sig")
        # 带BOM防止乱码
        for line in gachaLists[id]:
            f.write(line + "\n")
        f.close()


def writeXLSX(gachaLists, gachaTypeNames):
    import time

    import xlsxwriter

    t = time.strftime("%Y%m%d%H%M%S", time.localtime())
    # workbook = xlsxwriter.Workbook(f"{sys.path[0]}\\gacha-{t}.xlsx")
    workbook = xlsxwriter.Workbook("".format(str(sys.path[0]))+"gecha-".format(str(t))+".xlsx")
    for id in range(0, len(gachaTypeNames)):
        gachaList = gachaLists[id]
        gachaTypeName = gachaTypeNames[id]
        gachaList.reverse()
        header = "时间,编号,名称,类别,星级,总次数,保底内"
        worksheet = workbook.add_worksheet(gachaTypeName)
        content_css = workbook.add_format({"align": "left", "font_name": "微软雅黑", "border_color": "#c4c2bf", "border": 1})
        title_css = workbook.add_format({"align": "left", "font_name": "微软雅黑", "bg_color": "#ebebeb", "border_color": "#c4c2bf", "border": 1})
        excel_col = ["A", "B", "C", "D", "E", "F", "G"]
        excel_header = header.split(",")
        worksheet.set_column("A:A", 22)
        worksheet.set_column("C:C", 14)
        for i in range(len(excel_col)):
            # worksheet.write(f"{excel_col[i]}1", excel_header[i], border)
            # worksheet.write(f"{excel_col[i]}1", excel_header[i], title_css)
            worksheet.write("".format(str(excel_col[i]))+"1".format(str(excel_header[i])).format(title_css))
        idx = 0
        pdx = 0
        for i in range(len(gachaList)):
            idx = idx + 1
            pdx = pdx + 1
            excel_data = gachaList[i].split(",")
            excel_data.extend([idx, pdx])
            excel_data[1] = int(excel_data[1])
            excel_data[4] = int(excel_data[4])
            for j in range(len(excel_col)):
                # worksheet.write(f"{excel_col[j]}{i+2}", excel_data[j])
                # worksheet.write(f"{excel_col[j]}{i+2}", excel_data[j], content_css)
                worksheet.write("".format(str(excel_col[j])).format(str(i+2)).format(str(excel_data[j])).format(str(content_css)))
            if excel_data[4] == 5:
                pdx = 0

        star_5 = workbook.add_format({"bg_color": "#ebebeb", "color": "#bd6932", "bold": True})
        star_4 = workbook.add_format({"bg_color": "#ebebeb", "color": "#a256e1", "bold": True})
        star_3 = workbook.add_format({"bg_color": "#ebebeb"})
        # star_3 = workbook.add_format({"bg_color": "#ebebeb", "color": "#35aacc"})
        # worksheet.conditional_format(f"A2:G{len(gachaList)+1}", {"type": "formula", "criteria": "=$E2=5", "format": star_5})
        worksheet.conditional_format("A2:G".format(str(len(gachaList)+1)), {"type": "formula", "criteria": "=$E2=5", "format": "".format(str(star_5))})
        # worksheet.conditional_format(f"A2:G{len(gachaList)+1}", {"type": "formula", "criteria": "=$E2=4", "format": star_4})
        worksheet.conditional_format("A2:G".format(str(len(gachaList)+1)), {"type": "formula", "criteria": "=$E2=4", "format": "".format(str(star_4))})
        # worksheet.conditional_format(f"A2:G{len(gachaList)+1}", {"type": "formula", "criteria": "=$E2=3", "format": star_3})
        worksheet.conditional_format("A2:G".format(str(len(gachaList)+1)), {"type": "formula", "criteria": "=$E2=3", "format": "".format(str(star_3))})

    workbook.close()


if __name__ == "__main__":
    main()
