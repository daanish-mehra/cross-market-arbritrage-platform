'use client'

import { useEffect, useState } from 'react'
import MarketDataView from '@/components/MarketDataView'
import SpiderHeader from '@/components/SpiderHeader'
import SpiderSidebar from '@/components/SpiderSidebar'
import MarketDetailModal from '@/components/MarketDetailModal'
import TradeModal from '@/components/TradeModal'
import PortfolioView from '@/components/PortfolioView'
import ComingSoonView from '@/components/ComingSoonView'
import OpportunitiesView from '@/components/OpportunitiesView'

interface MarketData {
  market_id: string
  market: number
  event_name: string
  best_bid: number
  best_ask: number
  bid_size: number
  ask_size: number
}

interface ArbitrageOpportunity {
  event_id: string
  buy_market: number
  sell_market: number
  buy_price: number
  sell_price: number
  profit_percentage: number
  max_size: number
}

export default function Home() {
  const [marketData, setMarketData] = useState<Map<string, MarketData>>(new Map())
  const [connected, setConnected] = useState(false)
  const [selectedMarket, setSelectedMarket] = useState<MarketData | null>(null)
  const [detailModalOpen, setDetailModalOpen] = useState(false)
  const [tradeModalOpen, setTradeModalOpen] = useState(false)
  const [balance, setBalance] = useState(100.0) // Starting fake balance: $100
  const [activeTab, setActiveTab] = useState('dashboard')
  const [opportunities, setOpportunities] = useState<ArbitrageOpportunity[]>([])
  const [positions, setPositions] = useState<Array<{
    marketId: string
    eventName: string
    side: 'buy' | 'sell'
    amount: number
    price: number
    timestamp: Date
    market: number
  }>>([])
  const [tradeHistory, setTradeHistory] = useState<Array<{
    marketId: string
    side: 'buy' | 'sell'
    amount: number
    price: number
    timestamp: Date
  }>>([])
  const [latency, setLatency] = useState<number>(0)

  useEffect(() => {
    const ws = new WebSocket('ws://localhost:8080')
    
    // Buffers for throttled updates
    let marketDataBuffer = new Map<string, MarketData>()
    const THROTTLE_INTERVAL = 1000 // Update UI every 1 second
    
    // Throttled update function
    const flushUpdates = () => {
      if (marketDataBuffer.size > 0) {
        setMarketData(prev => {
          const newMap = new Map(prev)
          marketDataBuffer.forEach((value, key) => {
            newMap.set(key, value)
          })
          return newMap
        })
        marketDataBuffer.clear()
      }
    }
    
    // Set up interval to flush updates periodically
    const updateInterval = setInterval(flushUpdates, THROTTLE_INTERVAL)
    
    // Latency measurement via ping/pong
    let pingInterval: NodeJS.Timeout | null = null
    
    ws.onopen = () => {
      console.log('Connected to WebSocket server')
      setConnected(true)
      
      // Send ping every second to measure latency
      pingInterval = setInterval(() => {
        if (ws.readyState === WebSocket.OPEN) {
          const timestamp = Date.now()
          ws.send(JSON.stringify({ type: 'ping', timestamp }))
        }
      }, 1000)
    }
    
    ws.onmessage = (event) => {
      try {
        const message = JSON.parse(event.data)
        
        if (message.type === 'pong') {
          // Calculate latency from ping/pong round-trip
          if (message.timestamp) {
            const now = Date.now()
            const rtt = now - message.timestamp
            setLatency(rtt)
          }
        } else if (message.type === 'market_data') {
          marketDataBuffer.set(message.data.market_id, message.data)
        } else if (message.type === 'opportunity') {
          setOpportunities(prev => {
            // Update or add opportunity
            const existingIndex = prev.findIndex(
              opp => opp.event_id === message.data.event_id &&
                     opp.buy_market === message.data.buy_market &&
                     opp.sell_market === message.data.sell_market
            )
            
            const newOpp: ArbitrageOpportunity = {
              event_id: message.data.event_id,
              buy_market: message.data.buy_market,
              sell_market: message.data.sell_market,
              buy_price: message.data.buy_price,
              sell_price: message.data.sell_price,
              profit_percentage: message.data.profit_percentage / 100.0, // Convert from percentage to decimal
              max_size: message.data.max_size
            }
            
            if (existingIndex >= 0) {
              const updated = [...prev]
              updated[existingIndex] = newOpp
              return updated
            } else {
              return [...prev, newOpp]
            }
          })
        }
      } catch (e) {
        console.error('Error parsing WebSocket message:', e)
      }
    }
    
    ws.onerror = (error) => {
      console.error('WebSocket error:', error)
      setConnected(false)
      if (pingInterval) {
        clearInterval(pingInterval)
        pingInterval = null
      }
    }
    
    ws.onclose = () => {
      console.log('WebSocket connection closed')
      setConnected(false)
      if (pingInterval) {
        clearInterval(pingInterval)
        pingInterval = null
      }
    }
    
    // Cleanup
    return () => {
      clearInterval(updateInterval)
      if (pingInterval) {
        clearInterval(pingInterval)
      }
      flushUpdates()
      ws.close()
    }
  }, [])

  return (
    <div className="spider-container">
      <SpiderHeader connected={connected} />
      <div className="spider-main">
        <div className="grid-background"></div>
        <SpiderSidebar activeTab={activeTab} onTabChange={setActiveTab} latency={latency} />
        <main className="spider-content">
          {activeTab === 'dashboard' && (
            <OpportunitiesView opportunities={opportunities} />
          )}

          {activeTab === 'markets' && (
            <>
              <div className="page-header">
                <div>
                  <h1 className="page-title">Markets</h1>
                  <p className="page-subtitle">ALL AVAILABLE MARKETS // REAL-TIME PRICES</p>
                </div>
              </div>
              <MarketDataView 
                marketData={Array.from(marketData.values())}
                onChartClick={(market) => {
                  setSelectedMarket(market)
                  setDetailModalOpen(true)
                }}
                onTradeClick={(market) => {
                  setSelectedMarket(market)
                  setTradeModalOpen(true)
                }}
              />
            </>
          )}

          {activeTab === 'portfolio' && (
            <PortfolioView 
              positions={positions}
              balance={balance}
            />
          )}

          {activeTab === 'analytics' && (
            <ComingSoonView 
              title="Analytics"
              subtitle="ADVANCED CHARTS & METRICS // COMING SOON"
              icon="📉"
            />
          )}

          {activeTab === 'history' && (
            <ComingSoonView 
              title="Trade History"
              subtitle="COMPLETE TRADE LOG // COMING SOON"
              icon="🕐"
            />
          )}
        </main>
      </div>

      {/* Modals */}
      <MarketDetailModal
        market={selectedMarket}
        isOpen={detailModalOpen}
        onClose={() => setDetailModalOpen(false)}
      />

      <TradeModal
        market={selectedMarket}
        isOpen={tradeModalOpen}
        onClose={() => setTradeModalOpen(false)}
        balance={balance}
        onTrade={(marketId, side, amount, price) => {
          const cost = amount * price
          const market = selectedMarket
          
          if (side === 'buy') {
            setBalance(prev => prev - cost)
          } else {
            setBalance(prev => prev + cost)
          }
          
          // Add to positions
          if (market) {
            setPositions(prev => [...prev, {
              marketId,
              eventName: market.event_name,
              side,
              amount,
              price,
              timestamp: new Date(),
              market: market.market
            }])
          }
          
          // Add to trade history
          setTradeHistory(prev => [...prev, {
            marketId,
            side,
            amount,
            price,
            timestamp: new Date()
          }])
          
          console.log(`Trade executed: ${side.toUpperCase()} $${amount.toFixed(2)} at ${(price * 100).toFixed(2)}¢`)
        }}
      />
    </div>
  )
}
