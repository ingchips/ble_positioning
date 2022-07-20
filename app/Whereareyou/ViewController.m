//
//  ViewController.m
//  Whereareyou
//
//  Created by yfan on 2022/4/20.
//

#import "ViewController.h"
#import "MQTTClient.h"
#import "OneViewController.h"
#import "TwoViewController.h"
#import "ThreeViewController.h"

@interface ViewController (){
    int num;
    float x1;
    float y1;
    float z1;
    //平均值
    float x2;
    float y2;
    float z2;
    
    NSInteger connectStatus;
    
}
@property (nonatomic,strong)MQTTSession *session;
@property (weak, nonatomic) IBOutlet UITextField *urlTf;
@property (weak, nonatomic) IBOutlet UIButton *urlBtn;
@property (weak, nonatomic) IBOutlet UITextField *portTf;


@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    num = 0;
    connectStatus = 0;
}

- (IBAction)touchurlBtn:(id)sender {
    
    [self.urlTf resignFirstResponder];
    [self.portTf resignFirstResponder];
    // 请求mqtt信息
    [self loadMqtt];
}

- (void)loadMqtt{
    if (_urlTf.text.length > 0 && _portTf.text.length > 0){
        
        MQTTCFSocketTransport *transport = [[MQTTCFSocketTransport alloc] init];
        transport.host = _urlTf.text;//@"192.168.3.5";
        transport.port = [_portTf.text intValue];//1883;
        
        self.session = [[MQTTSession alloc] init];
        self.session.transport = transport;
        self.session.delegate = (id<MQTTSessionDelegate>)self;
        self.session.clientId = @"mqttx_08df18ee";
        [self.session connectAndWaitTimeout:1];
        [self.session addObserver:self forKeyPath:@"status" options:(NSKeyValueObservingOptionOld) context:nil];
        [self.session connectWithConnectHandler:^(NSError *error) {
            // Do some work
            NSLog(@"=============11111");
            if (!error) {
                //topic
                [self.session subscribeToTopic:@"ingchips/position_3d" atLevel:MQTTQosLevelAtMostOnce subscribeHandler:^(NSError *error, NSArray<NSNumber *> *gQoss) {
                    if (error) {
                        NSLog(@"Subscription failed %@", error.localizedDescription);
                    } else {
                        NSLog(@"Subscription sucessfull! Granted Qos: %@", gQoss);
                        UIAlertController *alertController = [UIAlertController alertControllerWithTitle:@"提示" message:nil preferredStyle:UIAlertControllerStyleAlert];
                        [alertController addAction: [UIAlertAction actionWithTitle:@"获取成功" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
                            
                        }]];
                        
                        [self presentViewController:alertController animated:YES completion:nil];
                    }
                 }];
            }
            
            
        }];
    }else{
        
        UIAlertController *alertController = [UIAlertController alertControllerWithTitle:@"提示" message:nil preferredStyle:UIAlertControllerStyleAlert];
        [alertController addAction: [UIAlertAction actionWithTitle:@"请输入地址已获取定位信息" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
            
        }]];
//        [alertController addAction: [UIAlertAction actionWithTitle:@"取消" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
//        }]];
        [self presentViewController:alertController animated:YES completion:nil];
        
    }
    
    
//    NSData *data = [@"hello" dataUsingEncoding:NSUTF8StringEncoding];
//
//    [self.session publishData:data onTopic:@"ingchips/position_3d" retain:YES qos:MQTTQosLevelAtMostOnce publishHandler:^(NSError *error) {
//    }];
}

//- (IBAction)turnOn:(id)sender {
//    NSData *data = [@"hello" dataUsingEncoding:NSUTF8StringEncoding];
//
//    [self.session publishData:data onTopic:@"ingchips/position_3d" retain:YES qos:MQTTQosLevelAtMostOnce publishHandler:^(NSError *error) {
//        NSLog(@"==========44444");
//    }];
//}
- (IBAction)touchOne:(id)sender {
    if (connectStatus != 1) {
        UIAlertController *alertController = [UIAlertController alertControllerWithTitle:@"提示" message:nil preferredStyle:UIAlertControllerStyleAlert];
        [alertController addAction: [UIAlertAction actionWithTitle:@"请输入地址已获取定位信息" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
        }]];
        [self presentViewController:alertController animated:YES completion:nil];
        return;
    }
    OneViewController *one = [[OneViewController alloc] init];
    self.delegate = one;
    [self.navigationController pushViewController:one animated:YES];
}

- (IBAction)touchTwo:(id)sender {
    if (connectStatus != 1) {
        UIAlertController *alertController = [UIAlertController alertControllerWithTitle:@"提示" message:nil preferredStyle:UIAlertControllerStyleAlert];
        [alertController addAction: [UIAlertAction actionWithTitle:@"请输入地址已获取定位信息" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
        }]];
        [self presentViewController:alertController animated:YES completion:nil];
        return;
    }
    TwoViewController *two = [[TwoViewController alloc] init];
    self.delegate = two;
    [self.navigationController pushViewController:two animated:YES];
}

- (IBAction)touchThree:(id)sender {
    if (connectStatus != 1) {
        UIAlertController *alertController = [UIAlertController alertControllerWithTitle:@"提示" message:nil preferredStyle:UIAlertControllerStyleAlert];
        [alertController addAction: [UIAlertAction actionWithTitle:@"请输入地址已获取定位信息" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
        }]];
        [self presentViewController:alertController animated:YES completion:nil];
        return;
    }
    ThreeViewController *three = [[ThreeViewController alloc] init];
    self.delegate = three;
    [self.navigationController pushViewController:three animated:YES];
}



- (void)startSubcribeMessageWithDeviceTheme:(NSString *)deviceTheme delegate:(id<MQTTSessionDelegate>)delegate {
    //这里会把订阅的VC设置为代理，这个定于的VC实现代理方法就可以收到推送的消息了
    self.session.delegate = delegate;
    if (self.session.status == MQTTSessionStatusConnected) {
        //订阅
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            dispatch_async(dispatch_get_main_queue(), ^{
                [self.session subscribeToTopic:deviceTheme atLevel:MQTTQosLevelAtMostOnce subscribeHandler:^(NSError *error, NSArray<NSNumber *> *gQoss) {
                    if (error) {
                        NSLog(@"订阅失败 %@", error.localizedDescription);
                    }
                    else {
                        NSLog(@"订阅成功 %@", gQoss);
                    }
                }];
            });
        });
    }
}

- (void)newMessage:(MQTTSession *)session data:(NSData *)data onTopic:(NSString *)topic qos:(MQTTQosLevel)qos retained:(BOOL)retained mid:(unsigned int)mid {
    // New message received in topic
    NSString *dataString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    NSLog(@"==================2222");
    NSLog(@"topic==================%@",topic);
    NSLog(@"dataString==================%@",dataString);
    NSDictionary *dic = [self dictionaryWithJsonString:dataString];
//    float x = [[dic objectForKey:@"x"] floatValue];
//    float y = [[dic objectForKey:@"y"] floatValue];
//    float z = [[dic objectForKey:@"z"] floatValue];
    
    //过滤
//    num +=1;
//    if (num <= 5) {
//        x1 = x1 + x;
//        y1 = y1 + y;
//        z1 = z1 + z;
//        if (num == 5) {
//            x2 = x1 / 5;
//            y2 = x1 / 5;
//            z2 = x1 / 5;
//            NSLog(@"=%f  =%f  =%f",x2,y2,z2);
//        }
//    }
//    else{
//        if (fabsf(x2-x)<0.1 && fabsf(y2-x)<0.1 && fabsf(z2-x)<0.1) {
//            NSLog(@"筛选后的坐标==================%@",dic);
//            if ([self.delegate respondsToSelector:@selector(PassValue:)]) {
//               [self.delegate PassValue:dic];
//            }
//        }
//
//    }
    
    if ([self.delegate respondsToSelector:@selector(PassValue:)]) {
       [self.delegate PassValue:dic];
    }
    
}
//# 数据过滤：前五个值取平均值，偏离大于0.1的过滤
//    print("num1：------------", num)
//    num = num + 1
//    print("num2：------------", num)
//    if num < 6:
//        x1 = x1 + coordinate[0][0]
//        y1 = y1 + coordinate[1][0]
//        z1 = z1 + coordinate[2][0]
//        if num == 5:
//            x2 = x1 / 5
//            y2 = x1 / 5
//            z2 = x1 / 5

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context {
    NSLog(@"=========333");
//    DebugLog(@"MQTT的状态:%@_____%@",self.session,change);
//    if (self.session.status == MQTTSessionStatusDisconnecting||MQTTSessionEventConnectionError) {
//    //重连即可
//        [self.session connect];
//    }
//    NSData *data = [@"hello" dataUsingEncoding:NSUTF8StringEncoding];
//
//    [self.session publishData:data onTopic:@"ingchips/position_3d" retain:YES qos:MQTTQosLevelAtMostOnce publishHandler:^(NSError *error) {
//        NSLog(@"==========44444");
//    }];
    
    
}
-(void)handleEvent:(MQTTSession *)session event:(MQTTSessionEvent)eventCode error:(NSError *)error{
    switch (eventCode) {
            case MQTTSessionEventConnected:
            connectStatus = 1;
                NSLog(@"连接成功");
            
                break;
            case MQTTSessionEventConnectionRefused:
                
                NSLog(@"连接被拒绝");
            [self.session connect];
//                self.isConnect = NO;
                break;
            case MQTTSessionEventConnectionClosed:
                
                NSLog(@"连接关闭");
            [self.session connect];
                
                break;
            case MQTTSessionEventConnectionError:
                
                NSLog(@"连接错误");
             
                break;
            case MQTTSessionEventProtocolError:
                
                NSLog(@"协议不被接受/协议错误");
            
                break;
            case MQTTSessionEventConnectionClosedByBroker:
                
                NSLog(@"其余错误");
            
                break;
                
            default:
                break;
        }
}
/**
 关闭MQTT
 

- (void)closeMQTT{

    //避免删除kvo异常
    @try{
        //清除kvo监听
        [self.manager removeObserver:self forKeyPath:@"state"];
    }
    @catch(NSException *exception){

    }

    //关闭连接 - 触发监听(有可能关成功,测试出有关失败的可能)
    [self.manager disconnect];

}
 */

- (NSDictionary *)dictionaryWithJsonString:(NSString *)jsonString{

    if (jsonString == nil) {

        return nil;

    }

    NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];

    NSError *err;

    NSDictionary *dic = [NSJSONSerialization JSONObjectWithData:jsonData

                                                        options:NSJSONReadingMutableContainers

                                                          error:&err];

    if(err) {

        NSLog(@"json解析失败：%@",err);

        return nil;

    }

    return dic;

}

@end
